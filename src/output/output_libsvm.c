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
 * @defgroup output 
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "util.h"
#include "output.h"
#include "murmur.h"

/* External variables */
extern config_t cfg;

/* Local variables */
static FILE *f = NULL;

/**
 * Opens a file for writing libsvm format
 * @param fn File name
 * @return number of regular files
 */
int input_libsvm_open(char *fn) 
{
    assert(fn);    
    
    f = fopen(fn, "w");
    if (!f) {
        error("Could not open output file '%s'.", fn);
        return FALSE;
    }
    
    /* Write sally header */
    sally_version(f, "#");
    fprintf(f, "# Output in LibSVM format");
    
    return TRUE;
}

/**
 * Writes a block of files to the output
 * @param x Feature vectors
 * @param len Length of block
 * @return number of written files
 */
int output_libsvm_write(fvec_t **x, int len)
{
    assert(x && len > 0);
    int j, i;

    for (j = 0; j < len; j++) {
        /* Print feature vector */
        fprintf(f, "%u ", x[j]->label);
        for (i = 0; i < x[j]->len; i++) 
            fprintf(f, "%llu:%f ", (long long unsigned int)  x[j]->dim[i] + 1, 
                    x[j]->val[i]);
        fprintf(f, "# ");

        /* Print source of string */
        if (x[j]->src)
            fprintf(f, "%s ", x[j]->src);
    
        /* Print string features */
        if (fhash_enabled()) {
            fprintf(f, "[");
            for (i = 0 ; i < x[j]->len; i++) {
                fhash_write_entry(f, fhash_get(x[j]->dim[i]));
                if (i < x[j]->len - 1)
                    fprintf(f, " ");
            }
            fprintf(f, "]\n");
        }
    
        fprintf(f, "\n");
    }
}

/**
 * Closes an open output file.
 */
void output_libsvm_close()
{
    if (f)
        fclose(f);
}

/** @} */
