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
 * @addtogroup fvec Feature vector
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "fvec.h"
#include "fmath.h"
#include "util.h"
#include "input.h"

/**
 * Normalizes a feature vector using a given normalization.
 * @param fv Feature vector
 * @param n normalization mode
 */
void fvec_norm(fvec_t *fv, const char *n)
{
    int i;
    double s = 0;
    
    if (!strcasecmp(n, "none")) {
        return;
    } else if (!strcasecmp(n, "l1")) {
        for (i = 0; i < fv->len; i++)
            s += fabs(fv->val[i]);
        for (i = 0; i < fv->len; i++)
            fv->val[i] = fv->val[i] / s;
    } else if (!strcasecmp(n, "l2")) {
        for (i = 0; i < fv->len; i++)
            s += pow(fv->val[i], 2);
        for (i = 0; i < fv->len; i++)
            fv->val[i] = fv->val[i] / sqrt(s);
    } else {
        warning("Unknown normalization mode '%s', using 'none'.", n);
    }
} 

/** @} */
