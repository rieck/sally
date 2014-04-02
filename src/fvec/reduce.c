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
 * Compute simhash from feature vector. The dimensions of the resulting 
 * vector correspond to the bits of the simhash.
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
 * Compute minhash from feature vector. The dimensions of the resulting 
 * vector correspond to the bits of the minhash.
 * @param fv Feature vector
 * @param num Number of bits
 */
void reduce_minhash(fvec_t *fv, int num)
{
    assert(fv && num > 0);
    feat_t *dim;
    float *val;
    int j;

    if (num > fv->len)
        num = fv->len;

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
        val[j] = fv->dim[j] & 1;
    }
    
    /* Exchange data */
    free(fv->dim);
    free(fv->val);
    
    fv->dim = dim;
    fv->val = val;
    fv->len = num;
}


/** @} */
