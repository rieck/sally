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
 * @addtogroup output 
 * Module 'matlab'.
 * <b>'matlab'</b>: The vectors are exported as a matlab file 
 * version 5. Each vector is stored as a sparse numeric array. 
 * If parameter <code>explicit_hash</code> is enabled in the
 * string features are stored in a separate cell array.
 * 
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "util.h"
#include "output.h"
#include "sally.h"
#include "fhash.h"

/* External variables */
extern config_t cfg;

/* Local variables */
static FILE *f = NULL;

/**
 * Opens a file for writing matlab format
 * @param fn File name
 * @return number of regular files
 */
int output_matlab_open(char *fn) 
{
    assert(fn);    
    
    f = fopen(fn, "w");
    if (!f) {
        error("Could not open output file '%s'.", fn);
        return FALSE;
    }
    
    /* Write matlab header */
    sally_version(f, "#");
    fprintf(f, "# Output module for matlab format\n");
    
    return TRUE;
}

/**
 * Writes a block of files to the output
 * @param x Feature vectors
 * @param len Length of block
 * @return number of written files
 */
int output_matlab_write(fvec_t **x, int len)
{
    assert(x && len >= 0);
    int j, i, k;

    for (j = 0; j < len; j++) {
    }
    
    return TRUE;
}

/**
 * Closes an open output file.
 */
void output_matlab_close()
{
    if (f)
        fclose(f);
}

/** @} */
