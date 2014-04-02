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
 * @defgroup reduce Dimension reduction
 *
 * Simple implementations of dimension reduction and related techniques.
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "fvec.h"
#include "util.h"

/**
 * Reduce the feature vector to a similarity hash. The string features
 * associated with each dimension are hashed and aggregated to a single hash
 * value as proposed by Charikar (STOC 2002).  For convenience, the computed
 * hash value is again represented as a feature vector.
 * 
 * @param fv Feature vector
 * @param num Number of bits
 */
void reduce_simhash(fvec_t *fv, int num)
{
    assert(fv && num > 0);
    feat_t *dim;
    float *val;
    int i, j;

    if (num > sizeof(feat_t) * 8)
        num = sizeof(feat_t) * 8;

    dim = (feat_t *) calloc(num, sizeof(feat_t));
    val = (float *) calloc(num, sizeof(float));
    
    if (!dim || !val) {
        error("Could not allocate feature vector contents");
        free(dim);
        free(val);
        return;
    }
  
    /* Compute aggregated feature hashes */
    for (i = 0; i < fv->len; i++) {
        feat_t hash = fv->dim[i];
        for (j = 0; j < num; j++) {
            dim[j] = j;
            if (hash & 1)
                val[j] += fv->val[i];
            else 
                val[j] -= fv->val[i];
            hash = hash >> 1;
        }
    }    
    
    /* Binarize feature hash */
    for (j = 0; j < num; j++)
        val[j] = val[j] > 0 ? 1 : 0;
        
    /* Exchange data */
    free(fv->dim);
    free(fv->val);
    
    fv->dim = dim;
    fv->val = val;
    fv->len = num;
}


/**
 * Reduce the feature vector to a "minimum hash". The string features
 * associated with each dimension are hashed and sorted.  In a slight
 * variant of the original idea by Broder (1997), the final hash is
 * constructed from the last bit of the k lowest hash values.  Hence, we are
 * not looking at the minimum hash values of k hash functions as in the
 * original formulation, but rather at k bits from the lowest k hash values
 * of one hash function. 
 *
 * Why? Well, in this way, we are constructing a hash value, whereas the 
 * original formulation returns a set of hash values.
 *
 * @param fv Feature vector
 * @param num Number of bits
 */
void reduce_minhash(fvec_t *fv, int num)
{
    assert(fv && num > 0);
    feat_t *dim;
    float *val;
    int j;

    dim = (feat_t *) calloc(num, sizeof(feat_t));
    val = (float *) calloc(num, sizeof(float));
    
    if (!dim || !val) {
        error("Could not allocate feature vector contents");
        free(dim);
        free(val);
        return;
    }
  
    /* Compute hash from smallest feature dims */
    for (j = 0; j < num; j++) {
        dim[j] = j;
        if (j < fv->len)
            val[j] = fv->dim[j] & 1;
        else
            val[j] = fv->dim[j] & 0;
    }
    
    /* Exchange data */
    free(fv->dim);
    free(fv->val);
    
    fv->dim = dim;
    fv->val = val;
    fv->len = num;
}

/** @} */
