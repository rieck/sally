/*
 * Sally - A Tool for Embedding Strings in Vector Spaces
 * Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */

/** 
 * @defgroup farray Array of feature vectors
 * Generic array of feature vectors. This module contains functions for
 * extracting and maintaining feature vectors in an array.
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "farray.h"
#include "fvec.h"
#include "fhash.h"
#include "util.h"

#ifdef ENABLE_LIBARCHIVE
#include <archive.h>
#include <archive_entry.h>
static void list_aentries(char *dir, int *fnum, int *total);
#endif

/* Local functions */
static char *load_file(char *path, char *name, int *size);
static void list_dentries(char *dir, int *fnum, int *total);
static char *file_suffix(char *file);

/*
 * Extracts the suffix from a file name. If the file does not
 * have a suffix, the function returns "unknown". 
 * @param file File name
 * @return pointer to suffix
 */
char *file_suffix(char *file)
{
    char *name = file + strlen(file) - 1;

    /* Determine dot in file name */
    while (name != file && *name != '.')
        name--; 

    /* Check for files with no suffix */
    if (name == file)
        name = "unknown";
    else
        name++;

    return name;
}


/**
 * Returns the number of entries in a directory. 
 * @param dir Directory to analyse
 * @param fnum Return pointer for number of regular files
 * @param total Return pointer for number of total entries
 */
static void list_dentries(char *dir, int *fnum, int *total)
{  
    struct dirent *dp;
    DIR *d;

    *fnum = 0;
    *total = 0;

    d = opendir(dir);
    while (d && (dp = readdir(d)) != NULL) {
        if (dp->d_type == DT_REG)
            ++*fnum;
        ++*total;
    }
    closedir(d);
}

#ifdef ENABLE_LIBARCHIVE
/**
 * Returns the number of file entries in an archive.
 * @param arc archive containing files
 * @param fnum Return pointer for number of regular files
 * @param total Return pointer for number of total files
 */
void list_aentries(char *arc, int *fnum, int *total)
{
    struct archive *a;
    struct archive_entry *entry;
    assert(arc);

    *fnum = 0;
    *total = 0;

    /* Open archive */
    a = archive_read_new();
    archive_read_support_compression_all(a);
    archive_read_support_format_all(a);
    archive_read_open_filename(a, arc, 4096);

    /* Jump through archive */
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const struct stat *s = archive_entry_stat(entry);
        if (S_ISREG(s->st_mode))
            ++ *fnum;

        ++*total;
        archive_read_data_skip(a);
    }
    archive_read_finish(a);
}
#endif


/**
 * Loads a  file into a byte array. The array is allocated 
 * and need to be free'd later by the caller.
 * @param path Path to file
 * @param name File name or NULL
 * @param size Pointer to file size
 * @return file data
 */
static char *load_file(char *path, char *name, int *size)
{
    assert(path);
    long read;
    char *x = NULL, file[512];
    struct stat st;

#pragma omp critical (snprintf)
    {
        /* snprintf is not necessary thread-safe. good to know. */
        if (name)
            snprintf(file, 512, "%s/%s", path, name);
        else
            snprintf(file, 512, "%s", path);
    }

    /* Open file */
    FILE *fptr = fopen(file, "r");
    if (!fptr) {
        error("Could not open file '%s'", file);
        return NULL;
    }

    /* Allocate memory */
    stat(file, &st);
    *size = st.st_size;
    if (!(x = malloc((*size + 1) * sizeof(char)))) {
        error("Could not allocate memory for file data");
        return NULL;
    }

    /* Read data */
    read = fread(x, sizeof(char), *size, fptr);
    fclose(fptr);
    if (*size != read)
        warning("Could not read all data from file '%s'", file);

    return x;
}



/**
 * Creates and allocates an empty array of feature vectors
 * @param s Source of array, e.g. directory
 * @return empty array
 */
farray_t *farray_create(char *s)
{
    farray_t *fa = calloc(1, sizeof(farray_t));
    if (!fa) {
        error("Could not allocate array of feature vectors");
        return NULL;
    }

    /* Init elements of array */
    fa->len = 0;

    /* Set source */
    if (s) 
        fa->src = strdup(s);

    return fa;
}

/**
 * Destroys an array of feature vectors
 * @param fa array of feature vectors
 */
void farray_destroy(farray_t *fa)
{
    if (!fa)
        return;

    /* Free feature vectors */
    if (fa->x) {
        for (int i = 0; i < fa->len; i++)
            fvec_destroy(fa->x[i]);
        free(fa->x);
    }

    if (fa->src)
        free(fa->src);

    free(fa);
}

/**
 * Adds a feature vector to the array
 * @param fa Feature array
 * @param fv Feature vector 
 */
void farray_add(farray_t *fa, fvec_t *fv)
{
    assert(fa && fv);

    /* Expand size of array */
    if (fa->len % BLOCK_SIZE == 0) {
        int l = fa->len + BLOCK_SIZE;
        if (!(fa->x = realloc(fa->x, l * sizeof(fvec_t *)))) {
            error("Could not re-size feature array");
            farray_destroy(fa);
            return;
        }
    }

    /* Update table */
    fa->x[fa->len++] = fv;
}


/**
 * Extracts an array of feature vectors from a directory. The function 
 * loads and converts files from the given directory. It does not process
 * subdirectories recursively.
 * @param path directory containing files
 * @param sa Sally structure
 * @return array of feature vectors
 */
farray_t *farray_extract_dir(char *path, sally_t *sa)
{
    assert(path && sa);

    int i, fnum, total, size;
    struct dirent *dp;

    /* Open directory */
    DIR *d = opendir(path);
    if (!d) {
        error("Could not open directory '%s'", path);
        return NULL;
    }
    
    /* Allocate empty array */
    farray_t *fa = farray_create(path);
    if (!fa)
        return NULL;

    /*
     * Prepare concurrent readdir_r(). There is a race condition in the 
     * following code. The maximum  length 'maxlen' could have changed 
     * between the previous call to opendir() and the following call to
     * pathconf(). I'll take care of this at a later time.
     */
    list_dentries(path, &fnum, &total);
    int maxlen = pathconf(path, _PC_NAME_MAX);

    /* Loop over directory entries */
#pragma omp parallel for shared(d,fa) private(dp) ordered
    for (i = 0; i < total; i++) {
        struct dirent *buf;
        buf = malloc(offsetof(struct dirent, d_name) + maxlen + 1);

        /* Read directory entry to local buffer */
        readdir_r(d, (struct dirent *) buf, &dp);
        /* Skip all entries except for regular files */
        if (dp->d_type != DT_REG) 
            goto skip;

        /* Load file content */
        printf("%s/%s %d of %d %d\n", path, dp->d_name, i, fnum, total);
        char *raw = load_file(path, dp->d_name, &size);
        if (!raw) 
            goto skip;
            
        printf("loaded %d\n", size);

        /* Extract feature vector from string */
        fvec_t *fv = fvec_extract(raw, size, sa);
        
        /* Set additional information */
        fvec_set_source(fv, dp->d_name);
        fvec_set_label(fv, file_suffix(dp->d_name));
        
#pragma omp critical (farray)
        farray_add(fa, fv);

        /* Clean string and directory buffer */
        free(raw);
skip:
        free(buf);
    }
    
    closedir(d);
    return fa;
}


#ifdef ENABLE_LIBARCHIVE
/**
 * Extracts an array of feature vectors from an archive. The function 
 * loads and converts files from the given archive. It does not process
 * subdirectories recursively.
 * @param path archive containing files
 * @param sa Sally configuration
 * @return array of feature vectors
 */
farray_t *farray_extract_arc(char *arc, sally_t *sa)
{
    assert(arc && sa);

    struct archive *a;
    struct archive_entry *entry;
    int i, fnum, total;
    char *raw, *name;

    /* Allocate empty array */
    farray_t *fa = farray_create(arc);
    if (!fa)
        return NULL;

    list_aentries(arc, &fnum, &total);

    /* Open archive */
    a = archive_read_new();
    archive_read_support_compression_all(a);
    archive_read_support_format_all(a);
    archive_read_open_filename(a, arc, 4096);

    /* Read contents */
#pragma omp parallel for shared(a) private(name,raw) ordered
    for (i = 0; i < total; i++) {
        long len = 0;
#pragma omp critical (farray)
        {
            /* Perform reading of archive in critical region */
            archive_read_next_header(a, &entry);
            const struct stat *s = archive_entry_stat(entry);
            len = s->st_size;
            if (!S_ISREG(s->st_mode)) {
                raw = NULL;
                archive_read_data_skip(a);
                name = NULL;
            } else {
                raw = malloc(len * sizeof(char));
                archive_read_data(a, raw, len);
                name = strdup((char *) archive_entry_pathname(entry));
            }
        }

        /* Skip non-regular files */
        if (!raw || !name)
            continue;
            
        /* Extract feature vector from string */
        fvec_t *fv = fvec_extract(raw, len, sa);
                
        /* Set additional information */
        fvec_set_source(fv, name);
        fvec_set_label(fv, file_suffix(name));
        
#pragma omp critical (farray)
        farray_add(fa, fv);
        free(raw);
        free(name);
    }

    /* Close archive */
    archive_read_finish(a);
    
    return fa;
}
#endif


/**
 * Prints a feature array
 * @param File pointer
 * @param fa feature array
 * @param sa Sally structure
 */
void farray_print(FILE *f, farray_t *fa, sally_t *sa)
{
    assert(fa);
    int i;

    printf("# Feature array [len: %lu, src: %s]\n", fa->len, fa->src);
    for (i = 0; i < fa->len; i++)
        fvec_print(f, fa->x[i], sa);
}


/**
 * Merges two arrays into one. The second array is destroy and all 
 * its memory is free'd.
 * @param x First array of feature vectors
 * @param y Second array of feature vectors
 * @return array of feature vectors
 */
farray_t *farray_merge(farray_t *x, farray_t *y)
{
    int i;

    /* Check for arguments */
    if (!x && y)
        return y;
    if (!y && x)
        return x;

    /* Add to old array */
    for (i = 0; i < y->len; i++) {
        farray_add(x, y->x[i]);
        y->x[i] = NULL;
    }

    /* Clean up */
    farray_destroy(y);
    return x;
}

/**
 * Exports feature vectors to libsvm format. 
 * @warning The dimension is incremented by one.
 * @param f File pointer
 * @param fa Array of feature vectors
 * @param sa Sally configuration
 */
void farray_to_libsvm(FILE *f, farray_t *fa, sally_t *sa)
{
    assert(f && fa);
    int i, j;
    
    sally_version(f);
    sally_print(f, sa);

    if (sa->fhash)
        fhash_print(f, sa->fhash);
    
    for (j = 0; j < fa->len; j++) {
        fprintf(f, "%u ", fa->x[j]->label);
        for (i = 0; i < fa->x[j]->len; i++) 
            fprintf(f, "%llu:%f ", (long long unsigned int) fa->x[j]->dim[i] + 1, 
                    fa->x[j]->val[i]);
        
        if (fa->x[j]->src)
            fprintf(f, "# %s", fa->x[j]->src);
        
        fprintf(f, "\n");    
    }
}


/** @} */
