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
    } else if (!strcasecmp(n, "bin")) {
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
 * Clones a feature vector. This function is useful for evaluation of
 * equations, e.g. x = y + 0, implies that x is a clone of y and not a
 * a reference.
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

/** 
 * Adds one feature vector to another (a = a + b)
 * @param fa Feature vector (a)
 * @param fb Feature vector (b)
 */
void fvec_add(fvec_t *fa, fvec_t *fb)
{
    unsigned long i = 0, j = 0, len = 0;
    assert(fa && fb);
    feat_t *dim;
    float *val;
    
    /* Allocate arrays */
    dim = (feat_t *) malloc((fa->len + fb->len) * sizeof(feat_t));
    val = (float *) malloc((fa->len + fb->len) * sizeof(float));
    if (!dim || !val) {
        error("Could not allocate feature vector contents");
        return;
    }
    
    /* Loop over features in a and b */
    while (i < fa->len && j < fb->len) {
        if (fa->dim[i] > fb->dim[j]) {
            dim[len] = fb->dim[j];
            val[len++] = (float) (fb->val[j++]);
        } else if (fa->dim[i] < fb->dim[j]) {
            dim[len] = fa->dim[i];
            val[len++] = fa->val[i++];
        } else {
            dim[len] = fa->dim[i];
            val[len++] = (float) (fa->val[i++] + fb->val[j++]);
        }
    }
    
    /* Loop over remaining features  */
    while (j < fb->len) {
        dim[len] = fb->dim[j];
        val[len++] = (float) fb->val[j++];
    }
    while (i < fa->len) {
        dim[len] = fa->dim[i];
        val[len++] = fa->val[i++];
    }
    
    /* Free old memory */
    free(fa->dim);
    free(fa->val);
    
    /* Update */
    fa->dim = dim;
    fa->val = val;
    fa->len = len;
    
    /* Reallocate memory */
    fvec_realloc(fa);
}

/** 
 * Element-wise multiplication of one feature vector with another (a = a x b)
 * @param fa Feature vector (a)
 * @param fb Feature vector (b)
 */
void fvec_times(fvec_t *fa, fvec_t *fb)
{
    unsigned long i = 0, j = 0, p, q, k;
    
    /* Loop over dimensions fa */
    for (i = 0, j = 0; j < fa->len; j++) {
        /* Binary search */
        p = i, q = fb->len;
        do {
            k = i, i = ((q - p) >> 1) + p;
            if (fb->dim[i] > fa->dim[j]) {
                q = i;
            } else if (fb->dim[i] < fa->dim[j]) {
                p = i;
            } else {
                fa->val[j] = fb->val[i] * fa->val[j];
                break;
            }
        } while (i != k);
        
        /* No match */
        if (i == k)
            fa->val[j] = 0;
    }
}


/** 
 * Dot product between two feature vectors (s = <a,b>). The function 
 * uses a binary search to sum over all dimensions.
 * @param fa Feature vector (a)
 * @param fb Feature vector (b)
 * @return s Inner product
 */
static double fvec_dot_bsearch(fvec_t *fa, fvec_t *fb)
{
    unsigned long i = 0, j = 0, p, q, k;
    double s = 0;
    
    /* Check if fa is larger than fb */
    if (fa->len < fb->len) {
        fvec_t *tmp = fa;
        fa = fb, fb = tmp;
    }
    
    /* Loop over dimensions fb */
    for (i = 0, j = 0; j < fb->len; j++) {
        /* Binary search */
        p = i, q = fa->len;
        do {
            k = i, i = ((q - p) >> 1) + p;
            if (fa->dim[i] > fb->dim[j]) {
                q = i;
            } else if (fa->dim[i] < fb->dim[j]) {
                p = i;
            } else {
                s += fa->val[i] * fb->val[j];
                break;
            }
        } while (i != k);
    }
    
    return s;
}

/** 
 * Dot product between two feature vectors (s = <a,b>). The function 
 * uses a loop to sum over all dimensions.
 * @param fa Feature vector (a)
 * @param fb Feature vector (b)
 * @return s Inner product
 */
static double fvec_dot_loop(fvec_t *fa, fvec_t *fb)
{
    unsigned long i = 0, j = 0;
    double s = 0;
    
    /* Loop over features in a and b */
    while (i < fa->len && j < fb->len) {
        if (fa->dim[i] > fb->dim[j]) {
            j++;
        } else if (fa->dim[i] < fb->dim[j]) {
            i++;
        } else {
            s += fa->val[i++] * fb->val[j++];
        }
    }
    
    return s;
}

/** 
 * Dot product between two feature vectors (s = <a,b>). The function 
 * uses a loop or a binary search to sum over all dimensions depending
 * on the size of the considered vectors. The vectors need to be 
 * normalized, that is, ||a|| = 1.
 * @param fa Feature vector (a)
 * @param fb Feature vector (b)
 * @return s Inner product
 */
double fvec_dot(fvec_t *fa, fvec_t *fb)
{
    assert(fa && fb);
    double a, b;
    
    /* Sort vectors according to size */
    if (fa->len > fb->len) {
        a = (double) fa->len, b = (double) fb->len;
    } else {
        b = (double) fa->len, a = (double) fb->len;
    }
    
    /* Choose dot functions */
    if (a + b > ceil(b * log2(a)))
        return fvec_dot_bsearch(fa, fb);
    else
        return fvec_dot_loop(fa, fb);
}


/**
 * Multiplies vector with a scalar (f = s * f)
 * @param f Feature vector 
 * @param s Scalar value
 */
void fvec_mul(fvec_t *f, double s)
{
    int i = 0;
    assert(f);
    
    for (i = 0; i < f->len; i++)
        f->val[i] = (float) (f->val[i] * s);
}


/** @} */
