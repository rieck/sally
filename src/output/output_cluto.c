/*
 * Sally - A Tool for Embedding Strings in Vector Spaces
 * Copyright (C) 2011 Konrad Rieck (konrad@mlsec.org)
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
 * <em>text</em>: The vectors are exported as a sparse matrix suitable for
 * the clustring tool Cluto. The first line is a header required by Cluto,
 * while the remaining lines are simply sparse representations of the
 * feature vectors. A detailed description of the format is available here:
 * http://www.cs.umn.edu/~karypis/cluto
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
static long rows = 0;
static long cols = 0;
static long entries = 0;
static int skip_null = CONFIG_FALSE;

/**
 * Opens a file for writing cluto format
 * @param fn File name
 * @return number of regular files
 */
int output_cluto_open(char *fn)
{
    assert(fn);
    cfg_int bits;

    f = fopen(fn, "w");
    if (!f) {
        error("Could not open output file '%s'.", fn);
        return FALSE;
    }

    config_lookup_bool(&cfg, "output.skip_null", &skip_null);
    config_lookup_int(&cfg, "features.hash_bits", (int *) &bits);
    cols = 1 << bits;

    /* Write dummy header. We fix it later */
    fprintf(f, "%12ld %12ld %12ld\n", rows, cols, entries);

    return TRUE;
}

/**
 * Writes a block of files to the output
 * @param x Feature vectors
 * @param len Length of block
 * @return number of written files
 */
int output_cluto_write(fvec_t **x, int len)
{
    assert(x && len >= 0);
    int j, i;

    for (j = 0; j < len; j++) {
        /* Skip empty vector */
        if (skip_null && x[j]->len == 0)
            continue;

        for (i = 0; i < x[j]->len; i++) {
            fprintf(f, "%llu ", (long long unsigned int) x[j]->dim[i] + 1);
            fprintf(f, "%g", x[j]->val[i]);
            if (i < x[j]->len - 1)
                fprintf(f, " ");
            entries++;
        }
        rows++;
        fprintf(f, "\n");
    }

    return TRUE;
}

/**
 * Closes an open output file.
 */
void output_cluto_close()
{
    /* Update header line */
    fseek(f, 0L, SEEK_SET);
    fprintf(f, "%12ld %12ld %12ld\n", rows, cols, entries);

    fclose(f);
}

/** @} */
