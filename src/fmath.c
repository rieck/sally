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
 * @defgroup fvec Math functions for feature vector
 *
 * Implementations of simple math functions for feature vectors, such as
 * normalizations.
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "fvec.h"
#include "fmath.h"
#include "util.h"

/**
 * Normalize a feature vector using a given normalization.
 * @param fv Feature vector
 * @param n normalization mode
 */
void fvec_norm(fvec_t *fv, norm_t n)
{
    int i;

    switch(n) {
        case NORM_L1:
            for (i = 0; i < fv->len; i++)
                fv->val[i] = fv->val[i] / fv->total;        
            break;
        case NORM_L2:
            for (i = 0; i < fv->len; i++)
                fv->val[i] = sqrt(fv->val[i] / fv->total);
            break;
    }
} 


/**
 * Binarize the components of a feature vector.
 * @param fv Feature vector
 */
void fvec_binarize(fvec_t *fv) 
{
    assert(fv);

    int i;
    for (i = 0; i < fv->len; i++)
        fv->val[i] = 1.0;
}

/** @} */
