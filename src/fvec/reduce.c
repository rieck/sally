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
 * Implementations of unsupervised filtering and dimension reduction methods.
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "fvec.h"
#include "util.h"
#include "reduce.h"

/* External variables */
extern config_t cfg;


/**
 * Dimension reduction wrapper.
 * @param fv Feature vector
 */
void dim_reduce(fvec_t *fv)
{
    const char *method;
    cfg_int dim_num;

    /* Get dimension reduction method */
    config_lookup_string(&cfg, "filter.dim_reduce", &method);
    config_lookup_int(&cfg, "filter.dim_num", &dim_num);

    if (!strcasecmp(method, "none")) {
        /* Do nothing ;) */
    } else if (!strcasecmp(method, "simhash")) {
        reduce_simhash(fv, dim_num);
    } else if (!strcasecmp(method, "minhash")) {
        reduce_minhash(fv, dim_num);
    } else if (!strcasecmp(method, "bloom")) {
        reduce_bloom(fv, dim_num);
    } else {
        warning("Unknown dimension reduction method. Skipping.");
    }

    /* Sparsify vector to reduce space */
    fvec_sparsify(fv);
}

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
    cfg_int hash_bits;

    config_lookup_int(&cfg, "features.hash_bits", &hash_bits);

    if (num > hash_bits)
        num = hash_bits;

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
            if (hash & 1)
                val[j] += fv->val[i];
            else
                val[j] -= fv->val[i];
            hash = hash >> 1;
        }
    }

    /* Set indices */
    for (j = 0; j < num; j++)
        dim[j] = j;

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
 * Reduce the feature vector to a minimum hash. The string features
 * associated with each dimension are hashed and sorted multiple times as
 * proposed by Broder (1997).  In each round the smallest hash value is
 * appended to the minimum hash.  
 *
 * @param fv Feature vector
 * @param num Number of bits
 */
void reduce_minhash(fvec_t *fv, int num)
{
    assert(fv && num > 0);
    feat_t *dim, min_hash = 0;
    float *val;
    int i, j, k;
    cfg_int hash_bits;

    config_lookup_int(&cfg, "features.hash_bits", &hash_bits);

    dim = (feat_t *) calloc(num, sizeof(feat_t));
    val = (float *) calloc(num, sizeof(float));

    if (!dim || !val) {
        error("Could not allocate feature vector contents");
        free(dim);
        free(val);
        return;
    }

    /* Fill hash bits */
    for (i = 0, j = 0; i < num; i++, j++) {

        /* Determine minimum hash value */
        if (i % hash_bits == 0) {
            min_hash = UINT64_MAX;
            for (k = 0; k < fv->len; k++) {
                /* Re-hash feature depending on round */
                feat_t h = rehash(fv->dim[k], i / hash_bits);
                h = h & ((1 << hash_bits) - 1);

                if (h < min_hash)
                    min_hash = h;
            }
            j = 0;
        }

        dim[i] = i;
        val[i] = (min_hash >> j) & 1;
    }

    /* Exchange data */
    free(fv->dim);
    free(fv->val);

    fv->dim = dim;
    fv->val = val;
    fv->len = num;
}


/**
 * Reduce the feature vector to a small Bloom filter.  The string features
 * associated with each dimension are hashed using k hash functions.  For
 * each string feature and each hash function one bit in the filter is set. 
 * As a result, the filter is populated similar to a real Bloom filter,
 * except for that the size of the filter is very small.
 *
 * @param fv Feature vector @param num Number of bits
 */
void reduce_bloom(fvec_t *fv, int num)
{
    assert(fv && num > 0);
    feat_t *dim;
    float *val;
    int i, k;
    cfg_int bloom_num;

    config_lookup_int(&cfg, "filter.bloom_num", &bloom_num);

    dim = (feat_t *) calloc(num, sizeof(feat_t));
    val = (float *) calloc(num, sizeof(float));

    if (!dim || !val) {
        error("Could not allocate feature vector contents");
        free(dim);
        free(val);
        return;
    }

    /* Fill dimension indices */
    for (i = 0; i < num; i++)
        dim[i] = i;

    /* Fill Bloom filter */
    for (i = 0; i < fv->len; i++) {
        for (k = 0; k < bloom_num; k++) {
            feat_t h = rehash(fv->dim[i], k);
            val[h % num] = 1.0;
        }
    }

    /* Exchange data */
    free(fv->dim);
    free(fv->val);

    fv->dim = dim;
    fv->val = val;
    fv->len = num;
}

/** @} */
