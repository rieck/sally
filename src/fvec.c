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

/* External variables */
extern int verbose;
extern config_t cfg;

/* Local functions */
static void extract_wgrams(fvec_t *, char *x, int l, int n, int b);
static void extract_ngrams(fvec_t *, char *x, int l, int n, int b);
static void count_feat(fvec_t *fv);
static int cmp_feat(const void *x, const void *y);
static void cache_put(fentry_t *c, fvec_t *fv, char *t, int l);
static void cache_flush(fentry_t *c, fvec_t *fv);

/* Delimiter functions and table */
static void decode_delim(const char *s);
static char delim[256] = { DELIM_NOT_INIT };

/**
 * Allocates and extracts a feature vector from a string.
 * @param x String of bytes (with space delimiters)
 * @param l Length of sequence
 * @return feature vector
 */
fvec_t *fvec_extract(char *x, int l)
{
    fvec_t *fv;
    long nlen, bits;
    const char *dlm_str, *cfg_str;
    assert(x && l >= 0);

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
    

    /* Get configuration */
    config_lookup_int(&cfg, "features.ngram_len", (long *) &nlen);
    config_lookup_string(&cfg, "features.ngram_delim", &dlm_str);
    config_lookup_int(&cfg, "features.hash_bits", (long *) &bits);

    /* N-grams of bytes */
    if (!dlm_str || strlen(dlm_str) == 0) {
        /* Feature extraction */
        extract_ngrams(fv, x, l, nlen, bits);
    } else {
        if (delim[0] == DELIM_NOT_INIT) {
            memset(delim, 0, 256);
            decode_delim(dlm_str);
        }

        /* Feature extraction */
        extract_wgrams(fv, x, l, nlen, bits);
    }

    /* Sort extracted features */
    qsort(fv->dim, fv->len, sizeof(feat_t), cmp_feat);

    /* Count features  */
    count_feat(fv);

    /* Compute embedding and normalization */
    config_lookup_string(&cfg, "features.vect_embed", &cfg_str);
    fvec_embed(fv, cfg_str);
    config_lookup_string(&cfg, "features.vect_norm", &cfg_str);
    fvec_norm(fv, cfg_str);

    return fv;
}

/**
 * Caches a feature for later addition to the feature hash table
 * @param c Pointer to cache
 * @param fv Feature vector
 * @param t Pointer to feature
 * @param l Length of feature
 */
static void cache_put(fentry_t *c, fvec_t *fv, char *t, int l) 
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
 */
static void cache_flush(fentry_t *c, fvec_t *fv) 
{
    int i; 
    
    /* Flush cache and add features to hash */
#pragma omp critical
    {
        for (i = 0; i < fv->len; i++) {
            fhash_put(c[i].key, c[i].data, c[i].len);
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
 * @param n N-gram length
 * @param b Bit of hash
 * @param d First delimiter
 */
static void extract_wgrams(fvec_t *fv, char *x, int l, int n, int b)
{
    assert(fv && x && l > 0);
    unsigned int i, j = l, k = 0, s = 0, q = 0, d;
    char *t = malloc(l + 1);
    fentry_t *cache = NULL;
#ifdef ENABLE_MD5HASH    
    unsigned char buf[MD5_DIGEST_LENGTH];
#endif    

    /* Set bits of hash mask */
    feat_t hash_mask = ((long long unsigned) 2 << (b - 1)) - 1; 

    if (fhash_enabled())
        cache = malloc(l * sizeof(fentry_t));

    /* Find first delimiter symbol */
    for (d = 0; !delim[(unsigned char) d] && d < 256; d++);

    /* Remove redundant delimiters */
    for (i = 0, j = 0; i < l; i++) {
        if (delim[(unsigned char) x[i]]) {
            if (j == 0 || delim[(unsigned char) t[j - 1]])
                continue;
            t[j++] = (char) d;
        } else {
            t[j++] = x[i];
        }
    }

    /* No characters remaining */
    if (j == 0) 
        goto clean;

    /* Add trailing delimiter */
    if (t[j - 1] != d)
        t[j++] = (char) d;

    /* Extract n-grams */
    for (k = i = 0; i < j; i++) {
        /* Count delimiters and remember start poisition*/
        if (t[i] == d && ++q == 1)      
                s = i;
        
        /* Store n-gram */
        if (q == n && i - k > 0) {

#ifdef ENABLE_MD5HASH        
            MD5((unsigned char *) (t + k), i - k, buf);
            memcpy(fv->dim + fv->len, buf, sizeof(feat_t));
#else            
            fv->dim[fv->len] = MurmurHash64B(t + k, i - k, 0x12345678); 
#endif            
            fv->dim[fv->len] &= hash_mask; 
            fv->val[fv->len] = 1;
            
            /* Cache feature and key */
            if (fhash_enabled()) 
                cache_put(cache, fv, t + k, i - k);
            
            k = s + 1, i = s, q = 0;
            fv->len++;
        }
    }

    /* Save extracted n-grams */
    fv->total = fv->len;

clean:    
    if (fhash_enabled()) {
        cache_flush(cache, fv);
        free(cache);
    }        
    free(t);    
}


/**
 * Extract byte n-grams from a string. The features (n-grams) are 
 * represented by hash values.
 * @param fv Feature vector
 * @param x Byte sequence 
 * @param l Length of sequence
 * @param n N-gram length
 * @param b Bits for hash
 */
static void extract_ngrams(fvec_t *fv, char *x, int l, int n, int b)
{
    assert(fv && x);

    unsigned int i = 0;
    char *t = x;
    fentry_t *cache = NULL;
#ifdef ENABLE_MD5HASH    
    unsigned char buf[MD5_DIGEST_LENGTH];
#endif    

    /* Set bits of hash mask */
    feat_t hash_mask = ((long long unsigned) 2 << (b - 1)) - 1; 

    if (fhash_enabled())
        cache = malloc(l * sizeof(fentry_t));

    for (i = 1; t < x + l; i++) {
        /* Check for sequence end */
        if (t + n > x + l)
            break;

#ifdef ENABLE_MD5HASH        
        MD5((unsigned char *) t, n, buf);
        memcpy(fv->dim + fv->len, buf, sizeof(feat_t));
#else            
        fv->dim[fv->len] = MurmurHash64B(t, n, 0x12345678); 
#endif    
        fv->dim[fv->len] &= hash_mask; 
        fv->val[fv->len] = 1;
        
        /* Cache feature */
        if (fhash_enabled())
            cache_put(cache, fv, t, n);
                
        t++;
        fv->len++;
    }
    fv->total = fv->len;
    
    if (!fhash_enabled())
        return;

    /* Flush cache */
    cache_flush(cache, fv);
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
void fvec_set_label(fvec_t *fv, float l)
{
    fv->label = l;
}

/**
 * Print the content of a feature vector
 * @param f File pointer
 * @param fv feature vector
 */
void fvec_print(FILE *f, fvec_t *fv)
{
    assert(fv);
    int i, j;

    fprintf(f, "Feature vector [src: %s, label: %g, len: %lu, total: %lu]\n", 
           fv->src, fv->label, fv->len, fv->total);
           
    for (i = 0; i < fv->len; i++) {
        fprintf(f, "   %.16llx:%6.4f [", (long long unsigned int) fv->dim[i], 
               fv->val[i]);

        if (fhash_enabled()) {
            fentry_t *fe = fhash_get(fv->dim[i]);
            for (j = 0; fe && j < fe->len; j++) 
                if (isprint(fe->data[j]) && !strchr("% ", fe->data[j]))
                    fprintf(f, "%c", fe->data[j]);
                else
                    fprintf(f, "%%%.2x", (unsigned char) fe->data[j]);
        }        
        
        fprintf(f, "]\n");        
    }    
}

/**
 * Loads a feature vector form a file stream
 * @param z File pointer
 * @return Feature vector
 */
fvec_t *fvec_load(gzFile *z)
{
    assert(z);
    fvec_t *f;
    char buf[512], str[512];
    int i, r;

    /* Allocate feature vector (zero'd) */
    f = calloc(1, sizeof(fvec_t));
    if (!f) {
        error("Could not load feature vector");
        return NULL;
    }

    gzgets(z, buf, 512);
    r = sscanf(buf, "fvec: len=%lu, total=%lu, label=%g, src=%s\n",
               (unsigned long *) &f->len, (unsigned long *) &f->total,
               (float *) &f->label, str);
    if (r != 4) 
        goto err;

    /* Set source */
    if (!strcmp(str, "(null)"))
        f->src = NULL;
    else
        f->src = strdup(str);

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
        gzgets(z, buf, 512);
        r = sscanf(buf, "  feat=%llx:%f\n", (unsigned long long *) &f->dim[i],
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
void fvec_save(fvec_t *f, gzFile *z)
{
    assert(f && z);
    int i;

    gzprintf(z, "fvec: len=%lu, total=%lu, label=%12.10f, src=lala\n",
             f->len, f->total, f->label, f->src ? f->src : "(null)");
    for (i = 0; i < f->len; i++)
        gzprintf(z, "  feat=%.16llx:%12.10f\n", (unsigned long long) f->dim[i],
                 (double) f->val[i]);
}


/**
 * Decodes a string containing delimiters to a global array
 * @param s String containing delimiters
 */
static void decode_delim(const char *s)
{
    char buf[5] = "0x00";
    unsigned int i, j;

    for (i = 0; i < strlen(s); i++) {
        if (s[i] != '%') {
            delim[(unsigned int) s[i]] = 1;
            continue;
        }

        /* Skip truncated sequence */
        if (strlen(s) - i < 2)
            break;

        buf[2] = s[++i];
        buf[3] = s[++i];
        sscanf(buf, "%x", (unsigned int *) &j);
        delim[j] = 1;
    }
}

/**
 * Resets delimiters table. There is a global table of delimiter 
 * symbols which is only initialized once the first sequence is 
 * processed. This functions is used to trigger a re-initialization.
 */
void fvec_reset_delim()
{
    delim[0] = DELIM_NOT_INIT;
}

/** @} */
