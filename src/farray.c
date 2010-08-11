/*
 * Sally - A Library for String Features and String Kernels
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
 * @author Konrad Rieck (konrad.rieck@tu-berlin.de)
 * @{
 */

#include "config.h"
#include "common.h"
#include "farray.h"
#include "fvec.h"
#include "md5.h"
#include "util.h"

/**
 * Compares two feature vectors using their source
 * @param x feature X
 * @param y feature Y
 * @return result as a signed integer
 */
static int cmp_fvec(const void *x, const void *y)
{
    fvec_t *fx = *((fvec_t **) x);
    fvec_t *fy = *((fvec_t **) y);

    if (fx->label > fy->label)
        return +1;
    if (fx->label < fy->label)
        return -1;

    return 0;
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
 * @param ja Sally configuration
 * @return array of feature vectors
 */
static farray_t *farray_extract_dir(char *path, sally_t *ja)
{
    assert(path && ja);

    int i, fnum, total;
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
#ifndef __MINGW32__    
    int maxlen = pathconf(path, _PC_NAME_MAX);
#endif    

    /* Loop over directory entries */
#pragma omp parallel for shared(d,fa) private(dp) ordered
    for (i = 0; i < total; i++) {

#ifdef __MINGW32__
        /* Read directory entry */
        dp = readdir(d);
#else
        struct dirent *buf;
        buf = malloc(offsetof(struct dirent, d_name) + maxlen + 1);

        /* Read directory entry to local buffer */
        readdir_r(d, (struct dirent *) buf, &dp);
        /* Skip directories */
        if (dp->d_type == DT_DIR) 
            goto skip;
#endif        
        /* Load file content */
        char *raw = load_file(path, dp->d_name);
        if (!raw) 
            goto skip;

        /* Extract feature vector from string */
        fvec_t *fv = fvec_extract(raw, strlen(raw), ja);
        
        /* Set additional information */
        fvec_set_label(fv, file_suffix(dp->d_name));
        
#pragma omp critical (farray)
        farray_add(fa, fv);

        /* Clean string and directory buffer */
        free(raw);
skip:
#ifndef __MINGW32__
        free(buf);
#else
        i = i + 1; /* Fake command */        
#endif        
    }
    
    closedir(d);
    return fa;
}

/**
 * Extracts an array of feature vectors from an archive or directory.
 * @param path archive or directory containing files
 * @param ja Cujo structure
 * @return array of feature vectors
 */
farray_t *farray_extract(char *path, sally_t *ja)
{
    assert(path && ja);

    struct stat st;
    farray_t *fa = NULL;

    if (stat(path, &st)) {
        error("Could not access file '%s'", path);
        return NULL;
    }

    if (S_ISDIR(st.st_mode))
        fa = farray_extract_dir(path, ja);
    else
        error("Unsupported file type of input '%s'", path);

    /* Sort feature array */
    if (fa)
        qsort(fa->x, fa->len, sizeof(fvec_t *), cmp_fvec);

    return fa;
}


/**
 * Prints a feature array
 * @param fa feature array
 */
void farray_print(farray_t *fa)
{
    assert(fa);
    int i;

    printf("# feature array [len: %lu, src: %s]\n", fa->len, fa->src);
    for (i = 0; i < fa->len; i++)
        fvec_print(fa->x[i]);
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

/** @} */
