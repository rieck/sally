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
 * <hr> 
 * <em>libsvm</em>: The vectors are exported in a sparse text
 * format which is used by LibSVM and SVMlight. Each vector is 
 * represented as a text line of the form
 * <pre> label dimension:value ... # source </pre>
 * Some version of LibSVM do not support comments, hence the comment
 * field may be manually cut off in a postprocessing step.
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
 * Opens a file for writing libsvm format
 * @param fn File name
 * @return number of regular files
 */
int output_libsvm_open(char *fn)
{
    assert(fn);

    f = fopen(fn, "w");
    if (!f) {
        error("Could not open output file '%s'.", fn);
        return FALSE;
    }

    /* Write sally header */
#if 0
    sally_version(f, "# ", "Output module for LibSVM format");
#endif

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
    assert(x && len >= 0);
    int j, i;

    for (j = 0; j < len; j++) {
#ifdef ENABLE_EVALTIME
        double t1 = time_stamp();
#endif

        /* Print feature vector */
        fprintf(f, "%g ", x[j]->label);
        for (i = 0; i < x[j]->len; i++)
            fprintf(f, "%llu:%g ", (long long unsigned int) x[j]->dim[i] + 1,
                    x[j]->val[i]);

        /* Print source of string */
        if (x[j]->src)
            fprintf(f, "# %s", x[j]->src);

        fprintf(f, "\n");

#ifdef ENABLE_EVALTIME
        printf("fdim %lu write %f\n", x[j]->len, time_stamp() - t1);
#endif
    }

    return TRUE;
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
