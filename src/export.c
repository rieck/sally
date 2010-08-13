/*
 * Sally - A Library for String Features and String Kernels
 * Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */

/** 
 * @defgroup fvec Export functions
 *
 * Functions for exporting feature vectors and other stuff.
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "util.h"
#include "fhash.h"
#include "export.h"

/**
 * Exports feature vectors to libsvm format. 
 * @warning The dimension is incremented by one.
 * @param f File pointer
 * @param sa Sally configuration
 * @param fv Array of feature vectors
 * @param l Length of array
 */
void export_fvec_libsvm(FILE *f, sally_t *sa, fvec_t **fv, int l)
{
    assert(f && fv);
    int i, j;
    
    sally_version(f);
    sally_print(f, sa);

    if (sa->fhash)
        fhash_print(f, sa->fhash);
    
    for (j = 0; j < l; j++) {
        fprintf(f, "%u ", fv[j]->label);
        for (i = 0; i < fv[j]->len; i++) 
            fprintf(f, "%llu:%f ", (long long unsigned int)  fv[j]->dim[i] + 1, 
                    fv[j]->val[i]);
        
        if (fv[j]->src)
            fprintf(f, "# %s", fv[j]->src);
        
        fprintf(f, "\n");    
    }
}

/** @} */
