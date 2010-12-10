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
 * <hr>
 * <em>lines</em>: The strings are stored as text lines in a file. Labels
 * are not supported by this input format.
 * @{
 */

#include "config.h"
#include "common.h"
#include "util.h"
#include "murmur.h"
#include "input.h"

/** Static variable */
static gzFile *in; 
static int line_num = 0;

/**
 * Opens a file for reading text lines. 
 * @param name File name
 * @return number of lines or -1 on error
 */
int input_lines_open(char *name) 
{
    assert(name);    

    in = gzopen(name, "r");
    if (!in) {
        error("Could not open '%s' for reading", name);
        return -1;
    }

    /* Count lines in file (I hope this is buffered)*/
    int c, num_lines = 0;
    do {
        c = gzgetc(in);
        if (c == '\n')
            num_lines++;
    } while(c != -1);

    /* Prepare reading */
    gzrewind(in);
    line_num = 1;

    return num_lines;
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
    int read, i = 0, j = 0;
    size_t size;
    char buf[32], *line = NULL;

    for (i = 0; i < len; i++) {
        line = NULL;        
        read = gzgetline(&line, &size, in);
        if (read == -1)  {
            free(line);
            break;
        }

        strs[j].str = line;
        strs[j].len = strlen(line);
        strs[j].label = 0;

        snprintf(buf, 32, "line%d", line_num++);
        strs[j].src = strdup(buf);
        j++;
    }
    
    return j;
}

/**
 * Closes an open directory.
 */
void input_lines_close()
{
    gzclose(in);
}

/** @} */
