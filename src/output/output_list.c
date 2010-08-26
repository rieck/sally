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
#include "sally.h"
#include "fhash.h"

/* External variables */
extern config_t cfg;

/* Local variables */
static FILE *f = NULL;

/**
 * Opens a file for writing list format
 * @param fn File name
 * @return number of regular files
 */
int output_list_open(char *fn) 
{
    assert(fn);    
    
    f = fopen(fn, "w");
    if (!f) {
        error("Could not open output file '%s'.", fn);
        return FALSE;
    }
    
    /* Write sally header */
    sally_version(f, "#");
    fprintf(f, "# Output module for list format\n");
    
    return TRUE;
}

/**
 * Writes a block of files to the output
 * @param x Feature vectors
 * @param len Length of block
 * @return number of written files
 */
int output_list_write(fvec_t **x, int len)
{
    assert(x && len >= 0);
    int j, i, k;

    for (j = 0; j < len; j++) {
        fprintf(f, "%g ", x[j]->label);
        for (i = 0; i < x[j]->len; i++) {

            /* Print feature (hash and string) */
            fentry_t *fe = fhash_get(x[j]->dim[i]);
            fprintf(f, "%llu:", (long long unsigned int)  x[j]->dim[i]);
            for (k = 0; fe && k < fe->len; k++) {
                if (isprint(fe->data[k]) && !strchr("%: ", fe->data[k])) 
                    fprintf(f, "%c", fe->data[k]);
                else
                    fprintf(f, "%%%.2x", (unsigned char) fe->data[k]);
            }

            /* Print value of feature */
            fprintf(f, ":%g", x[j]->val[i]);
            if (i < x[j]->len - 1)
                fprintf(f, ",");
        }

        /* Print source of string */
        if (x[j]->src)
            fprintf(f, " %s", x[j]->src);
    
        fprintf(f, "\n");
    }
    
    return TRUE;
}

/**
 * Closes an open output file.
 */
void output_list_close()
{
    if (f)
        fclose(f);
}

/** @} */
