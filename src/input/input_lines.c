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
 * @addtogroup input 
 *
 * Module <em>lines</em>: The strings are stored as text lines in a file.
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "util.h"
#include "murmur.h"
#include "input.h"

static float get_label(char *desc);

/**
 * Opens a file for reading text lines. 
 * @param name File name
 * @return number of lines or -1 on error
 */
int input_lines_open(char *name) 
{
    assert(name);    
    struct lineshive_entry *entry;

    a = lineshive_read_new();
    lineshive_read_support_compression_all(a);
    lineshive_read_support_format_all(a);
    int r = lineshive_read_open_filename(a, name, 4096);
    if (r != 0) {
        error("%s", lineshive_error_string(a));
        return -1;
    }

    /* Count regular files in lineshive */
    int num_files = 0;
    while (lineshive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const struct stat *s = lineshive_entry_stat(entry);
        if (S_ISREG(s->st_mode))
            num_files++;
        lineshive_read_data_skip(a);
    }
    lineshive_read_finish(a);
    
    /* Open file again */
    a = lineshive_read_new();
    lineshive_read_support_compression_all(a);
    lineshive_read_support_format_all(a);
    lineshive_read_open_filename(a, name, 4096);
    return num_files;
}

/**
 * Reads a block of files into memory.
 * @param strs Array for data
 * @param len Length of block
 * @return number of read files
 */
int input_lines_read(string_t *strs, int len)
{
    assert(strs && len > 0);
    struct lineshive_entry *entry;
    int i, j = 0;
    
    /* Load block of files (no OpenMP here)*/
    for (i = 0; i < len; i++) {    
        /* Perform reading of lineshive */
        int r = lineshive_read_next_header(a, &entry);
        if (r != ARCHIVE_OK)
            break;
        
        const struct stat *s = lineshive_entry_stat(entry);
        if (!S_ISREG(s->st_mode)) {
            lineshive_read_data_skip(a);
        } else {
            /* Add entry */
            strs[j].str = malloc(s->st_size * sizeof(char));
            lineshive_read_data(a, strs[j].str, s->st_size);
            strs[j].src = strdup(lineshive_entry_pathname(entry));
            strs[j].len = s->st_size;
            strs[j].label = get_label(strs[j].src);
            j++;
        }
    }
    
    return j;
}

/**
 * Closes an open directory.
 */
void input_lines_close()
{
    lineshive_read_finish(a);
}


/** 
 * Converts a file name to a label. The label is computed from the 
 * file's suffix; either directly if the suffix is a number or 
 * indirectly by hashing.
 * @param desc Description (file name) 
 * @return label value.
 */
static float get_label(char *desc)
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
