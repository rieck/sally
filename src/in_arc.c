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
 * @defgroup input Input functions
 * Implementation of various functions for reading data.
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "util.h"
#include "murmur.h"

#ifdef ENABLE_LIBARCHIVE

#include <archive.h>
#include <archive_entry.h>
#include "in_arc.h"

/* Local variables */
static struct archive *a = NULL;

/**
 * Opens an archive for reading files. 
 * @param name Archive name
 * @return number of regular files
 */
int input_arc_open(char *name) 
{
    assert(name);    
    struct archive_entry *entry;

    a = archive_read_new();
    archive_read_support_compression_all(a);
    archive_read_support_format_all(a);
    int r = archive_read_open_filename(a, name, 4096);
    if (r != 0) {
        error("%s", archive_error_string(a));
        return 0;
    }

    /* Count regular files in archive */
    int num_files = 0;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const struct stat *s = archive_entry_stat(entry);
        if (S_ISREG(s->st_mode))
            num_files++;
        archive_read_data_skip(a);
    }
    archive_read_finish(a);
    
    /* Open file again */
    a = archive_read_new();
    archive_read_support_compression_all(a);
    archive_read_support_format_all(a);
    archive_read_open_filename(a, name, 4096);
    return num_files;
}

/**
 * Reads a block of files into memory.
 * @param sa Sally configuration
 * @param data Array for data
 * @param sizes Array for sizes
 * @param desc Array for descriptions
 * @param len Length of block
 * @return number of read files
 */
int input_arc_read(char **data, int *sizes, char **desc, int len)
{
    assert(data && sizes && desc);
    struct archive_entry *entry;
    int i, j = 0;
    
    /* Load block of files (no OpenMP here)*/
    for (i = 0; i < len; i++) {    
        /* Perform reading of archive */
        archive_read_next_header(a, &entry);
        const struct stat *s = archive_entry_stat(entry);
        if (!S_ISREG(s->st_mode)) {
            archive_read_data_skip(a);
            continue;
        }
        
        /* Add entry */
        data[j] = malloc(s->st_size * sizeof(char));
        archive_read_data(a, data[j], s->st_size);
        desc[j] = strdup(archive_entry_pathname(entry));
        sizes[j] = s->st_size;
        j++;
    }
    
    return j;
}

/**
 * Closes an open directory.
 */
void input_arc_close()
{
    archive_read_finish(a);
}

/** 
 * Converts a file name to a label. The label is computed from the 
 * file's suffix; either directly if the suffix is a number or 
 * indirectly by hashing.
 * @param desc Description (file name) 
 * @return label value.
 */
float input_arc_desc2label(char *desc)
{
    char *endptr;
    char *name = desc + strlen(desc) - 1;

    /* Determine dot in file name */
    while (name != desc && *name != '.')
        name--; 

    /* Place pointer before '.' */
    if (name != desc)
        name++;

    /* Test direct conversion */
    float f = strtof(name, &endptr);
    
    /* Compute hash value */
    if (!endptr || strlen(endptr) > 0) 
        f = MurmurHash64B(name, strlen(name), 0xc0d3bab3) % 0xffff;
    
    return f;
} 

#endif

/** @} */
