/*
 * Sally - A Tool for Embedding Strings in Vector Spaces
 * Copyright (C) 2010-2012 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */

#include "config.h"
#include "common.h"
#include "util.h"
#include "output.h"
#include "sally.h"
#include "fhash.h"

/* Local variables */
static FILE* f = NULL;

/**
 * Opens a file for writing libsvm format
 * @param fn File name
 * @return number of regular files
 */
int output_scores_open(char* fn)
{
    assert(fn);    
    
    f = fopen(fn, "w+");
    if (f == NULL)
    {
        error("Could not open output file '%s'.", fn);
        return FALSE;
    }
    return TRUE;
}

/**
 * Writes a block of scores to the output
 * @param scores The score values
 * @param len Length of block
 * @return number of written lines
 */
int output_scores_write(const double* const scores, int len)
{
    assert(scores != NULL && len >= 0);

    for (int i = 0; i < len; i++)
    {
		fprintf(f, "%f\n", scores[i]);
    }
    return TRUE;
}

int output_dummy_write(fvec_t** x, int len)
{
	fatal("The scores module is used in an incorrect way. Go complain about it at the guy who implemented this.");
	return FALSE;
}

/**
 * Closes the open output file.
 */
void output_scores_close()
{
    if (f != NULL)
    {
        fclose(f);
    }
}

/** @} */
