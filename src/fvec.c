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
 * @defgroup fvec Feature vector
 *
 * Generic implementation of a feature vector. A feature vector
 * contains a sparse representation of non-zero dimensions in the
 * feature space. This allows for operating with vectors of high and
 * even infinite dimensionality, as long as the association between
 * dimensions and non-zero values is sparse. 
 *
 * The dimensions are indexed using hash values up to 64 bit. 
 * Depending on later application, the bit size of the hash function
 * may be reduced and colliding dimensions will be added. 
 * (See 'Hash Kernels', Shi et al. AISTATS 2009)
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "fvec.h"
#include "fhash.h"
#include "fmath.h"
#include "util.h"
#include "md5.h"
#include "murmur.h"
#include "sally.h"

/* Local functions */
static void extract_wgrams(fvec_t *, char *x, int l, int d, sally_t *);
static void extract_ngrams(fvec_t *, char *x, int l, sally_t *);
static void count_feat(fvec_t *fv);
static int cmp_feat(const void *x, const void *y);
static void cache_put(fentry_t *c, fvec_t *fv, sally_t *sa, char *t, int l);
static void cache_flush(fentry_t *c, fvec_t *fv, sally_t *sa);


/**
 * Allocates and extracts a feature vector from a string.
 * @param x String of bytes (with space delimiters)
 * @param l Length of sequence
 * @param sa Sally configuration
 * @return feature vector
 */
fvec_t *fvec_extract(char *x, int l, sally_t *sa)
{
    fvec_t *fv;
    int d;
    assert(x && sa && l >= 0);

    /* Allocate feature vector */
    fv = calloc(1, sizeof(fvec_t));
    if (!fv) {
        error("Could not extract feature vector");
        return NULL;
    }

    /* Initialize feature vector */
    fv->len = 0;
    fv->total = 0;
    fv->dim = (feat_t *) malloc(l * sizeof(feat_t));
    fv->val = (float *) malloc(l * sizeof(float));
    fv->src = NULL;

    /* Check for empty sequence */
    if (l == 0)
        return fv;

    if (!fv->dim || !fv->val) {
        error("Could not allocate feature vector");
        fvec_destroy(fv);
        return NULL;
    }

    /* Find first delimiter symbol */
    for (d = 0; !DELIM(sa, d) && d < 256; d++);
    
    /* Check for byte or word n-grams */
    if (d == 256)
        extract_ngrams(fv, x, l, sa);
    else
        extract_wgrams(fv, x, l, d, sa);

    /* Sort extracted features */
    qsort(fv->dim, fv->len, sizeof(feat_t), cmp_feat);

    /* Count features  */
    count_feat(fv);
    
    /* Binarize if requested */
    if (sa->embed == EMBED_BIN)
        fvec_binarize(fv);

    /* Compute normalization */
    fvec_norm(fv, sa->norm);    
    
    return fv;
}

/**
 * Caches a feature for later addition to the feature hash table
 * @param c Pointer to cache
 * @param fv Feature vector
 * @param sa Sally configuration
 * @param t Pointer to feature
 * @param l Length of feature
 */
static void cache_put(fentry_t *c, fvec_t *fv, sally_t *sa, char *t, int l) 
{
    c[fv->len].len = l;
    c[fv->len].key = fv->dim[fv->len];
    c[fv->len].data = malloc(l);
    if (c[fv->len].data)
        memcpy(c[fv->len].data, t, l);
    else
        error("Could not allocate feature data");
}    

/**
 * Flushs all features from the cache to the feature hash table.
 * @param c Pointer to cache
 * @param fv Feature vector
 * @param sa Sally configuration
 */
static void cache_flush(fentry_t *c, fvec_t *fv, sally_t *sa) 
{
    int i; 
    
    /* Flush cache and add features to hash */
#pragma omp critical
    {
        for (i = 0; i < fv->len; i++) {
            fhash_put(sa->fhash, c[i].key, c[i].data, c[i].len);
            free(c[i].data);
        }
    }
}

/**
 * Extracts word n-grams from a string. The features are represented 
 * by hash values.
 * @param fv Feature vector
 * @param x Byte sequence 
 * @param l Length of sequence
 * @param d First delimiter
 * @param sa Sally configuration 
 */
static void extract_wgrams(fvec_t *fv, char *x, int l, int d, sally_t *sa)
{
    assert(fv && x &&  sa && l > 0);
    unsigned int i, j = l, k = 0, s = 0, n = 0, o = 0;
    char *t = malloc(l + 1);
    fentry_t *cache = NULL;
#ifdef ENABLE_MD5HASH    
    unsigned char buf[MD5_DIGEST_LENGTH];
#endif    

    /* Set bits of hash mask */
    feat_t hash_mask = ((long long unsigned) 2 << (sa->bits - 1)) - 1; 

    if (sa->fhash)
        cache = malloc(l * sizeof(fentry_t));

    /* Remove redundant delimiters */
    for (i = 0, j = 0; i < l; i++) {
        if (DELIM(sa, x[i])) {
            if (j == 0 || DELIM(sa, t[j - 1]))
                continue;
            t[j++] = (char) d;
        } else {
            t[j++] = x[i];
        }
    }

    /* No characters remaining */
    if (j == 0)
        return;

    /* Add trailing delimiter */
    if (t[j - 1] != d)
        t[j++] = (char) d;

    /* Extract n-grams */
    for (k = i = o = 0; i < j; i++) {
    
        /* Count delimiters */
        if (t[i] == d) {
            n++;
            if (n == 1)          /* Remember first starting point */
                s = i;
            o = i;               /* Remember word starting point */
        } 
        
        /* Store n-gram */
        if (n == sa->nlen && i - k > 0) {

#ifdef ENABLE_MD5HASH        
            MD5((unsigned char *) (t + k), i - k, buf);
            memcpy(fv->dim + fv->len, buf, sizeof(feat_t));
#else            
            fv->dim[fv->len] = MurmurHash64B(t + k, i - k, 0x12345678); 
#endif            
            fv->dim[fv->len] &= hash_mask; 
            fv->val[fv->len] = 1;
            
            /* Cache feature and key */
            if (sa->fhash) 
                cache_put(cache, fv, sa, (t + k) , i - k);
            
            k = s + 1, i = s, n = 0, o = s;
            fv->len++;
        }
    }
    free(t);

    /* Save extracted n-grams */
    fv->total = fv->len;
    
    if (!sa->fhash)
        return;

    /* Flush cache */
    cache_flush(cache, fv, sa);
    free(cache);    
}


/**
 * Extract byte n-grams from a string. The features (n-grams) are 
 * represented by hash values.
 * @param fv Feature vector
 * @param x Byte sequence 
 * @param l Length of sequence
 * @param sa Sally structure
 */
static void extract_ngrams(fvec_t *fv, char *x, int l, sally_t *sa)
{
    assert(fv && x && sa);

    unsigned int i = 0;
    char *t = x;
    fentry_t *cache = NULL;
#ifdef ENABLE_MD5HASH    
    unsigned char buf[MD5_DIGEST_LENGTH];
#endif    

    /* Set bits of hash mask */
    feat_t hash_mask = ((long long unsigned) 2 << (sa->bits - 1)) - 1; 

    if (sa->fhash)
        cache = malloc(l * sizeof(fentry_t));

    for (i = 1; t < x + l; i++) {
        /* Check for sequence end */
        if (t + sa->nlen > x + l)
            break;

#ifdef ENABLE_MD5HASH        
        MD5((unsigned char *) t, sa->nlen, buf);
        memcpy(fv->dim + fv->len, buf, sizeof(feat_t));
#else            
        fv->dim[fv->len] = MurmurHash64B(t, sa->nlen, 0x12345678); 
#endif    
        fv->dim[fv->len] &= hash_mask; 
        fv->val[fv->len] = 1;
        
        /* Cache feature */
        if (sa->fhash)
            cache_put(cache, fv, sa, t, sa->nlen);
                
        t++;
        fv->len++;
    }
    
    if (!sa->fhash)
        return;

    /* Flush cache */
    cache_flush(cache, fv, sa);
    free(cache);   
}


/**
 * Compares two features values (hashs)
 * @param x feature X
 * @param y feature Y
 * @return result as a signed integer
 */
static int cmp_feat(const void *x, const void *y)
{
    if (*((feat_t *) x) > *((feat_t *) y))
        return +1;
    if (*((feat_t *) x) < *((feat_t *) y))
        return -1;
    return 0;
}

/** 
 * Counts featues in a preliminary feature vector
 * @param fv Valid feature vector
 */
static void count_feat(fvec_t *fv)
{
    feat_t *p_dim = fv->dim;
    float n = 0, *p_val = fv->val;
    unsigned int i;

    /* Loop over features */
    for (i = 0; i < fv->len; i++) {
        /* Skip zero values */
        if (fabs(fv->val[i]) < 1e-12)
            continue;

        /* Check for duplicate dims */
        if (i < fv->len - 1 && fv->dim[i] == fv->dim[i + 1]) {
            n += fv->val[i];
        } else {
            *(p_dim++) = fv->dim[i];
            *(p_val++) = fv->val[i] + n;
            n = 0;
        }
    }

    /* Update length */
    fv->len = p_dim - fv->dim;

    /* Reallocate memory */
    fvec_realloc(fv);
}

/**
 * Shrinks the memory of a feature vector. The function reallocates
 * the memory of features and its values, such that the required space
 * is reduced to a minimum.
 */
void fvec_realloc(fvec_t *fv)
{
    feat_t *p_dim;
    float *p_val;

    /*
     * Explicit reallocation. Don't use realloc(). On some platforms 
     * realloc() will not shrink memory blocks or copy to smaller sizes.
     * Consequently, realloc() may result in memory leaks. 
     */
    p_dim = malloc(fv->len * sizeof(feat_t));
    p_val = malloc(fv->len * sizeof(float));
    if (!p_dim || !p_val) {
        error("Could not re-allocate feature vector");
        free(p_dim);
        free(p_val);
        return;
    }

    /* Copy to new feature vector */
    memcpy(p_dim, fv->dim, fv->len * sizeof(feat_t));
    memcpy(p_val, fv->val, fv->len * sizeof(float));

    /* Free old */
    free(fv->dim);
    free(fv->val);
    fv->val = p_val;
    fv->dim = p_dim;
}

/**
 * Destroys a feature vector 
 * @param fv Feature vector
 */
void fvec_destroy(fvec_t *fv)
{
    if (!fv)
        return;
    if (fv->dim)
        free(fv->dim);
    if (fv->val)
        free(fv->val);
    if (fv->src)
        free(fv->src);        
    free(fv);
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

/**
 * Sets the source of a feature vector
 * @param fv Feature vector
 * @param s Source of features
 */
void fvec_set_source(fvec_t *fv, char *s)
{
    fv->src = strdup(s);
}

/**
 * Sets the label of a feature vector
 * @param fv Feature vector
 * @param l Label string
 */
void fvec_set_label(fvec_t *fv, char *l)
{
    unsigned char buf[MD5_DIGEST_LENGTH];
    char *endptr = NULL;

    fv->label = (unsigned int) strtol(l, &endptr, 10);
    
    if (!endptr || strlen(endptr) > 0) {
        MD5((unsigned char *) l, strlen(l), buf);
        memcpy(&(fv->label), buf, sizeof(fv->label));
    }
}

/**
 * Print the content of a feature vector
 * @param f File pointer
 * @param fv feature vector
 * @param sa Sally configuration
 */
void fvec_print(FILE *f, fvec_t *fv, sally_t *sa)
{
    assert(fv);
    int i, j;

    fprintf(f, "# Feature vector [src: %s, label: %u, len: %lu, total: %lu]\n", 
           fv->src, fv->label, fv->len, fv->total);
           
    for (i = 0; i < fv->len; i++) {
        fprintf(f, "#   %.16llx:%6.4f [", (long long unsigned int) fv->dim[i], 
               fv->val[i]);

        if (sa && sa->fhash) {
            fentry_t *fe = fhash_get(sa->fhash, fv->dim[i]);        
            for (j = 0; f && j < fe->len; j++) {
                if (isprint(fe->data[j]) || fe->data[j] == '%')
                    fprintf(f, "%c", fe->data[j]);
                else
                    fprintf(f, "%%%.2x", fe->data[j]);
            }
        }
        
        fprintf(f, "]\n");        
    }    
}

/**
 * Loads a feature vector form a file stream
 * @param f File pointer
 * @return Feature vector
 */
fvec_t *fvec_load(FILE *z)
{
    assert(z);
    fvec_t *f;
    char buf[512];
    int i, r;

    /* Allocate feature vector (zero'd) */
    f = calloc(1, sizeof(fvec_t));
    if (!f) {
        error("Could not load feature vector");
        return NULL;
    }

    fgets(buf, 512, z);
    r = sscanf(buf, "fvec: len=%lu, total=%lu, label=%hu\n",
               (unsigned long *) &f->len, (unsigned long *) &f->total,
               (unsigned short *) &f->label);
    if (r != 3) 
        goto err;

    /* Load src string */
    f->src = calloc(sizeof(char), 256);
    if (fscanf(z, "  %255[^\n]\n", f->src) != 1) 
        goto err;

    /* Empty feature vector */
    if (f->len == 0)
        return f;

    /* Allocate arrays */
    f->dim = (feat_t *) malloc(f->len * sizeof(feat_t));
    f->val = (float *) malloc(f->len * sizeof(float));
    if (!f->dim || !f->val) {
        error("Could not allocate feature vector contents");
        fvec_destroy(f);
        return NULL;
    }
    
    /* Load features */
    for (i = 0; i < f->len; i++) {
        fgets(buf, 512, z);
        r = sscanf(buf, "  %llx:%g\n", (unsigned long long *) &f->dim[i],
                   (float *) &f->val[i]);
        if (r != 2) 
            goto err;
    }

    return f;
err:
    error("Failed to parse feature vector");
    fvec_destroy(f);
    return NULL;
}


/**
 * Saves a feature vector to a file stream
 * @param f Feature vector
 * @param z File pointer
 */
void fvec_save(fvec_t *f, FILE * z)
{
    assert(f && z);
    int i;

    fprintf(z, "fvec: len=%lu, total=%lu, label=%hu\n",
             f->len, f->total, f->label);
    fprintf(z, "  %s\n", f->src);
    for (i = 0; i < f->len; i++)
        fprintf(z, "  %.16llx:%.16g\n", (unsigned long long) f->dim[i],
                 (float) f->val[i]);
}


/** @} */
