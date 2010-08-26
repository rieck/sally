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
 * @defgroup fvec 
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "fvec.h"
#include "fmath.h"
#include "util.h"

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
            s += fv->val[i];
        for (i = 0; i < fv->len; i++)
            fv->val[i] = fv->val[i] / s;
    } else if (!strcasecmp(n, "l2")) {
        for (i = 0; i < fv->len; i++)
            s += pow(fv->val[i], 2);
        for (i = 0; i < fv->len; i++)
            fv->val[i] = fv->val[i] / sqrt(s);
    } else {
        warning("Unknown normalization mode '%s', using 'l1'.", n);
        for (i = 0; i < fv->len; i++)
            fv->val[i] = fv->val[i] / fv->total;    
    }
} 

/**
 * Embeds a feature vector using a given normalization.
 * @param fv Feature vector
 * @param n normalization mode
 */
void fvec_embed(fvec_t *fv, const char *n)
{
    int i;
    
    if (!strcasecmp(n, "cnt")) {
        /* Nothing */
    } else if (!strcasecmp(n, "l2")) {
        for (i = 0; i < fv->len; i++)
            fv->val[i] = 1;
    } else {
        warning("Unknown embedding mode '%s', using 'cnt.", n);
    }
} 

/**
 * Binarizes the components of a feature vector.
 * @param fv Feature vector
 */
void fvec_binarize(fvec_t *fv) 
{
    assert(fv);
    
    int i;
    for (i = 0; i < fv->len; i++)
        fv->val[i] = 1.0;
}


/**
 * Clones a feature vector
 * @param o Feature vector
 * @return Cloned feature vector
 */
fvec_t *fvec_clone(fvec_t *o)
{
    assert(o);
    fvec_t *fv;
    unsigned int i;
    
    /* Allocate feature vector */
    fv = calloc(1, sizeof(fvec_t));
    if (!fv) {
        error("Could not clone feature vector");
        return NULL;
    }
    
    /* Clone structure */
    fv->len = o->len;
    fv->total = o->total;
    
    if (o->src)
        fv->src = strdup(o->src);
    
    /* Check for empty sequence */
    if (o->len == 0)
        return fv;
    
    fv->dim = (feat_t *) malloc(o->len * sizeof(feat_t));
    fv->val = (float *) malloc(o->len * sizeof(float));
    if (!fv->dim || !fv->val) {
        error("Could not allocate feature vector");
        fvec_destroy(fv);
        return NULL;
    }
    
    for (i = 0; i < o->len; i++) {
        fv->dim[i] = o->dim[i];
        fv->val[i] = o->val[i];
    }
    
    return fv;
}


/** @} */
