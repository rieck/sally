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
 * <em>text</em>: The vectors are exported in a sparse 
 * text format. Each vector is represented as a list of dimensions
 * which is written to a text file in the following form
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

/* Local variables */
static FILE *f = NULL;
static int skip_null = CONFIG_FALSE;

/**
 * Opens a file for writing text format
 * @param fn File name
 * @return number of regular files
 */
int output_text_open(char *fn)
{
    assert(fn);

    f = fopen(fn, "w");
    if (!f) {
        error("Could not open output file '%s'.", fn);
        return FALSE;
    }

    config_lookup_bool(&cfg, "output.skip_null", &skip_null);

    /* Write sally header */
    sally_version(f, "# ", "Output module for text format");

    return TRUE;
}

/**
 * Writes a block of files to the output
 * @param x Feature vectors
 * @param len Length of block
 * @return number of written files
 */
int output_text_write(fvec_t **x, int len)
{
    assert(x && len >= 0);
    int j, i, k;

    for (j = 0; j < len; j++) {
        /* Skip null vectors */
        if (skip_null && x[j]->len == 0)
            continue;
    
        for (i = 0; i < x[j]->len; i++) {
            /* Print feature (hash and string) */
            fentry_t *fe = fhash_get(x[j]->dim[i]);
            fprintf(f, "%llu:", (long long unsigned int) x[j]->dim[i] + 1);
            for (k = 0; fe && k < fe->len; k++) {
                if (isprint(fe->data[k]) && !strchr("%:, ", fe->data[k]))
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
            fprintf(f, " # %s", x[j]->src);

        fprintf(f, "\n");
    }

    return TRUE;
}

/**
 * Closes an open output file.
 */
void output_text_close()
{
    if (f)
        fclose(f);
}

/** @} */
