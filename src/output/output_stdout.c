/*
 * Sally - A Tool for Embedding Strings in Vector Spaces
 * Copyright (C) 2014 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */

/** 
 * @addtogroup output
 * <hr>
 * <em>stdout</em>: The vectors are exported in a sparse 
 * text format. Each vector is represented as a list of dimensions
 * and written to standard output in the following form
 * <pre> dimension:feature:value,... source </pre>
 * If parameter <code>explicit_hash</code> is not enabled in the
 * configuration, the field <code>feature</code> will be empty.
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

/**
 * Opens a file for writing stdout format
 * @param fn File name
 * @return number of regular files
 */
int output_stdout_open(char *fn)
{
    assert(fn);

    if (!stdout) {
        error("Could not open <stdout>");
        return FALSE;
    }

    /* Write sally header */
    sally_version(stdout, "# ", "Output module for stdout format");

    return TRUE;
}

/**
 * Writes a block of files to the output
 * @param x Feature vectors
 * @param len Length of block
 * @return number of written files
 */
int output_stdout_write(fvec_t **x, int len)
{
    assert(x && len >= 0);
    int j, i, k;

    for (j = 0; j < len; j++) {
        for (i = 0; i < x[j]->len; i++) {
            /* Print feature (hash and string) */
            fentry_t *fe = fhash_get(x[j]->dim[i]);
            fprintf(stdout, "%llu:", (long long unsigned int) x[j]->dim[i] + 1);
            for (k = 0; fe && k < fe->len; k++) {
                if (isprint(fe->data[k]) && !strchr("%:, ", fe->data[k]))
                    fprintf(stdout, "%c", fe->data[k]);
                else
                    fprintf(stdout, "%%%.2x", (unsigned char) fe->data[k]);
            }

            /* Print value of feature */
            fprintf(stdout, ":%g", x[j]->val[i]);
            if (i < x[j]->len - 1)
                fprintf(stdout, ",");
        }

        /* Print source of string */
        if (x[j]->src)
            fprintf(stdout, " %s", x[j]->src);

        fprintf(stdout, "\n");
    }

    return TRUE;
}

/**
 * Closes an open output file.
 */
void output_stdout_close()
{
    /* Do nothing */
}

/** @} */
