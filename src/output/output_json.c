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
 * <em>json</em>: The vectors are exported in JSON format. 
 * Each vector is represented as an objects, where its dimensions
 * values and features are represented by properties.
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
 * Opens a file for writing json format
 * @param fn File name
 * @return number of regular files
 */
int output_json_open(char *fn)
{
    assert(fn);

    f = fopen(fn, "w");
    if (!f) {
        error("Could not open output file '%s'.", fn);
        return FALSE;
    }

    config_lookup_bool(&cfg, "output.skip_null", &skip_null);

    /* Write sally header */
    fprintf(f, "[\n");

    return TRUE;
}

/**
 * Writes a block of files to the output
 * @param x Feature vectors
 * @param len Length of block
 * @return number of written files
 */
int output_json_write(fvec_t **x, int len)
{
    assert(x && len >= 0);
    int j, i, k;

    for (j = 0; j < len; j++) {
        /* Skip null vectors */
        if (skip_null && x[j]->len == 0)
            continue;

        fprintf(f, "  {\n");

        fprintf(f, "    \"dim\": [");
        for (i = 0; i < x[j]->len; i++) {
            fprintf(f, "%llu", (long long unsigned int) x[j]->dim[i] + 1);
            if (i < x[j]->len - 1)
                fprintf(f, ", ");
        }
        fprintf(f, "],\n");

        fprintf(f, "    \"val\": [");
        for (i = 0; i < x[j]->len; i++) {
            fprintf(f, "%g", x[j]->val[i]);
            if (i < x[j]->len - 1)
                fprintf(f, ", ");
        }
        fprintf(f, "]");

        /* Print feature if feature hash is enabled */
        if (fhash_enabled()) {
            fprintf(f, ",\n    \"feat\": [");
            for (i = 0; i < x[j]->len; i++) {
                /* Print feature (hash and string) */
                fentry_t *fe = fhash_get(x[j]->dim[i]);

                fprintf(f, "\"");
                for (k = 0; k < fe->len; k++)
                    if (isprint(fe->data[k]) && !strchr("%\"\\", fe->data[k]))
                        fprintf(f, "%c", fe->data[k]);
                    else
                        fprintf(f, "%%%.2x", (unsigned char) fe->data[k]);
                fprintf(f, "\"");

                if (i < x[j]->len - 1)
                    fprintf(f, ", ");
            }
            fprintf(f, "]");
        }

        /* Print source of string */
        if (x[j]->src)
            fprintf(f, ",\n    \"src\": \"%s\"", x[j]->src);

        fprintf(f, "\n  },\n");
    }

    return TRUE;
}

/**
 * Closes an open output file.
 */
void output_json_close()
{
    fseek(f, -2, SEEK_CUR);
    fprintf(f, "\n]\n");
    fclose(f);
}

/** @} */
