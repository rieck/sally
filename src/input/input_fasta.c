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
 * Module 'fasta'.
 * <b>'fasta'</b>: The strings are stored in FASTA format.
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "util.h"
#include "input.h"

/** Static variable */
static gzFile *in; 

/**
 * Opens a file for reading text fasta. 
 * @param name File name
 * @return number of fasta or -1 on error
 */
int input_fasta_open(char *name) 
{
    assert(name);    
    size_t read, size;
    char *line = NULL;

    in = gzopen(name, "r");
    if (!in) {
        error("Could not open '%s' for reading", name);
        return -1;
    }

    int num, cont = FALSE;
    while(!gzeof(in)) {
        line = NULL;
        read = gzgetline(&line, &size, in);
        if (read > 0)
            strtrim(line);
        if (read > 1 && !cont && (line[0] == '>' || line[0] == ';')) {
            num++;
            cont = TRUE;
        } else {
            cont = FALSE;
        }
        free(line);
    }

    /* Prepare reading */
    gzrewind(in);
    
    return num;
}

/**
 * Reads a block of files into memory.
 * @param strs Array for data
 * @param len Length of block
 * @return number of read files
 */
int input_fasta_read(string_t *strs, int len)
{
    assert(strs && len > 0);
    int read, i = 0, j = 0, alloc = -1;
    size_t size;
    char *line = NULL, *seq = NULL;

    for (i = 0; i < len; i++) {
        /* Read line */
        line = NULL;        
        read = gzgetline(&line, &size, in);

        /* Trim line */
        if (read >= 0)
            strtrim(line);
        
        /* Check for end or comment char */
        if (read == -1 || line[0] == ';' || line[0] == '>') {
            if (alloc > 1) {
                strs[j].str = seq;
                strs[j].len = alloc - 1;
                j++;
            }
                        
            if (alloc == -1 || alloc > 1) {
                if (read != -1) {
                    strs[j].src = strdup(line);
                    strs[j].label = 0.0;
                }
                seq = calloc(sizeof(char), 1);
                alloc = 1;
            }
            goto skip;
        } 
        
        if (alloc == -1)
            goto skip;
        
        alloc += strlen(line);
        seq = realloc(seq, alloc * sizeof(char));
        strlcat(seq, line, alloc);        
skip:        
        free(line);
    }
    
    return j;
}

/**
 * Closes an open directory.
 */
void input_fasta_close()
{
    gzclose(in);
}

/** @} */
