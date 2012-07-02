/*
 * Sally - A Tool for Embedding Strings in Vector Spaces
 * Copyright (C) 2010-2012 Konrad Rieck (konrad@mlsec.org)
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
#include "norm.h"
#include "embed.h"

/* External variables */
extern int verbose;
extern config_t cfg;

/* Local functions */
static void extract_wgrams(fvec_t *, char *x, int l);
static void extract_ngrams(fvec_t *, char *x, int l);
static void count_feat(fvec_t *fv);
static int cmp_feat(const void *x, const void *y);
static void cache_put(fentry_t *c, fvec_t *fv, char *t, int l);
static void cache_flush(fentry_t *c, fvec_t *fv);
static feat_t hash_str(char *s, int l);

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
    config_lookup_string(&cfg, "features.ngram_delim", &dlm_str);

    /* N-grams of bytes */
    if (!dlm_str || strlen(dlm_str) == 0) {
        /* Feature extraction */
        extract_ngrams(fv, x, l);
    } else {

#ifdef ENABLE_OPENMP
#pragma omp critical (delim)
#endif
        {
            if (delim[0] == DELIM_NOT_INIT)
                decode_delim(dlm_str);
        }

        /* Feature extraction */
        extract_wgrams(fv, x, l);
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
 * Allocates and extracts an empty feature vector
 * @return feature vector
 */
fvec_t *fvec_zero()
{
    return fvec_extract("", 0);
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
#ifdef ENABLE_OPENMP
#pragma omp critical
#endif
    {
        for (i = 0; i < fv->len; i++) {
            fhash_put(c[i].key, c[i].data, c[i].len);
            free(c[i].data);
        }
    }
}


/**
 * Hashes a string to a feature dimension. Utility function to limit
 * the clatter of code.
 * @param s Byte sequence
 * @param l Length of sequence
 * @return hash value
 * 
 */
static feat_t hash_str(char *s, int l)
{
    feat_t ret = 0;

#ifdef ENABLE_MD5HASH
    unsigned char buf[MD5_DIGEST_LENGTH];
#endif

#ifdef ENABLE_MD5HASH
    MD5((unsigned char *) s, l, buf);
    memcpy(&ret, buf, sizeof(feat_t));
#else
    ret = MurmurHash64B(s, l, 0x12345678);
#endif

    return ret;
}

/**
 * Compares two characters (I bet there is a similar function somewhere in 
 * libc and this is just some ugly code).
 * @param v1 first char 
 * @param v2 second char
 * @return comparisong result as integer
 */
static int chrcmp(const void *v1, const void *v2)
{
    char *c1 = (char *) v1;
    char *c2 = (char *) v2;

    if (*c1 > *c2)
        return +1;
    if (*c1 < *c2)
        return -1;
    return 0;
}

/**
 * Compares two words (not necessary null terminated)
 * @param v1 first word 
 * @param v2 second word
 * @return comparisong result as integer
 */
static int wordcmp(const void *v1, const void *v2)
{
    int l, c;
    word_t *w1 = (word_t *) v1;
    word_t *w2 = (word_t *) v2;

    if (w1->l > w2->l)
        l = w2->l;
    else
        l = w1->l;

    c = memcmp(w1->w, w2->w, l);
    if (c == 0)
        c = w1->l - w2->l;
    return c;
}

/**
 * Sorts the words in a string for sorted n-grams of words. A new string
 * is allocated and the original one is freed.
 * @param str string
 * @param len Length of string
 * @param delim Delimiter for words
 * @param return sorted string 
 */
static char *sort_words(char *str, int len, char delim)
{
    assert(str);
    assert(len > 0);

    int i, j, k;
    word_t words[len];

    /* Extract words */
    for (i = k = 0, j = 0; i < len; i++) {
        if (str[i] == delim || i == len - 1) {
            words[k].w = str + j;
            words[k].l = i - j;
            if (i == len - 1)
                words[k].l++;

            j = i + 1;
            k++;
        }
    }

    /* Sort words */
    qsort(words, k, sizeof(word_t), wordcmp);

    /* Allocate string with slack */
    char *s = malloc(len + sizeof(unsigned long));
    for (i = j = 0; i < k; i++) {
        memcpy(s + j, words[i].w, words[i].l);
        j += words[i].l + 1;
        s[j - 1] = delim;
    }

    free(str);
    return s;
}

/**
 * Extracts word n-grams from a string. The features are represented 
 * by hash values.
 * @param fv Feature vector
 * @param x Byte sequence 
 * @param l Length of sequence
 */
static void extract_wgrams(fvec_t *fv, char *x, int l)
{
    assert(fv && x && l > 0);
    int nlen, pos, sort, bits, sign, flen;
    unsigned int i, j = l, k = 0, s = 0, q = 0, d;
    char *t = malloc(l + 1), *fstr;
    fentry_t *cache = NULL;

    /* Get configuration */
    config_lookup_int(&cfg, "features.ngram_len", (int *) &nlen);
    config_lookup_int(&cfg, "features.ngram_pos", (int *) &pos);
    config_lookup_int(&cfg, "features.ngram_sort", (int *) &sort);
    config_lookup_int(&cfg, "features.hash_bits", (int *) &bits);
    config_lookup_int(&cfg, "features.vect_sign", (int *) &sign);

    /* Set bits of hash mask */
    feat_t hash_mask = ((long long unsigned) 2 << (bits - 1)) - 1;

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
        /* Count delimiters and remember start poisition */
        if (t[i] == d && ++q == 1)
            s = i;

        /* Store n-gram */
        if (q == nlen && i - k > 0) {
            /* Copy feature string and add slack */
            flen = i - k;
            fstr = malloc(flen + sizeof(unsigned long));
            memcpy(fstr, t + k, flen);

            /* Sorted n-grams code */
            if (sort)
                fstr = sort_words(fstr, flen, d);

            /* Positonal n-grams code */
            if (pos) {
                memcpy(fstr + flen, &(fv->len), sizeof(unsigned long));
                flen += sizeof(unsigned long);
            }

            feat_t h = hash_str(fstr, flen);
            fv->dim[fv->len] = h & hash_mask;
            fv->val[fv->len] = 1;

            /* Signed embedding */
            if (sign)
                fv->val[fv->len] *= (signed) h > 0 ? -1 : 1;

            /* Cache feature and key */
            if (fhash_enabled())
                cache_put(cache, fv, fstr, flen);

            k = s + 1, i = s, q = 0;
            fv->len++;
            free(fstr);
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
 */
static void extract_ngrams(fvec_t *fv, char *x, int l)
{
    assert(fv && x);

    unsigned int i = 0;
    int nlen, pos, sort, bits, flen, sign;
    char *fstr, *t = x;
    fentry_t *cache = NULL;

    /* Get configuration */
    config_lookup_int(&cfg, "features.ngram_len", (int *) &nlen);
    config_lookup_int(&cfg, "features.ngram_pos", (int *) &pos);
    config_lookup_int(&cfg, "features.ngram_sort", (int *) &sort);
    config_lookup_int(&cfg, "features.hash_bits", (int *) &bits);
    config_lookup_int(&cfg, "features.vect_sign", (int *) &sign);

    /* Set bits of hash mask */
    feat_t hash_mask = ((long long unsigned) 2 << (bits - 1)) - 1;

    if (fhash_enabled())
        cache = malloc(l * sizeof(fentry_t));

    for (i = 1; t < x + l; i++) {
        /* Check for sequence end */
        if (t + nlen > x + l)
            break;

        /* Copy feature string and add slack */
        flen = nlen;
        fstr = malloc(flen + sizeof(unsigned long));
        memcpy(fstr, t, nlen);

        /* Sorted n-grams code */
        if (sort)
            qsort(fstr, flen, 1, chrcmp);

        /* Positonal n-grams code */
        if (pos) {
            memcpy(fstr + flen, &(fv->len), sizeof(unsigned long));
            flen += sizeof(unsigned long);
        }

        feat_t h = hash_str(fstr, flen);
        fv->dim[fv->len] = h & hash_mask;
        fv->val[fv->len] = 1;

        /* Signed embedding */
        if (sign)
            fv->val[fv->len] *= (signed) h > 0 ? -1 : 1;

        /* Cache feature */
        if (fhash_enabled())
            cache_put(cache, fv, fstr, flen);

        t++;
        fv->len++;
        free(fstr);
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
void fvec_print(FILE * f, fvec_t *fv)
{
    assert(fv);
    int i, j;

    fprintf(f, "Feature vector [src: %s, label: %g, len: %lu, total: %lu]\n",
            fv->src, fv->label, fv->len, fv->total);

    for (i = 0; i < fv->len; i++) {
        fprintf(f, "   %.16llx:%6.4f [", (long long unsigned int) fv->dim[i] +1,
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
 * Reads a feature vector form a file stream
 * @param z File pointer
 * @return Feature vector
 */
fvec_t *fvec_read(gzFile z)
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

        // The output representation of the dimension is increased by one, cf. fvec_write(.,.)
        f->dim[i]--;
    }

    return f;
  err:
    error("Failed to parse feature vector");
    fvec_destroy(f);
    return NULL;
}


/**
 * Writes a feature vector to a file stream
 * @param f Feature vector
 * @param z File pointer
 */
void fvec_write(fvec_t *f, gzFile z)
{
    assert(f && z);
    int i;

    gzprintf(z, "fvec: len=%lu, total=%lu, label=%12.10f, src=%s\n",
             f->len, f->total, f->label, f->src ? f->src : "(null)");
    for (i = 0; i < f->len; i++)
        gzprintf(z, "  feat=%.16llx:%12.10f\n",
                 (unsigned long long) f->dim[i] +1, (double) f->val[i]);
}

/**
 * Loads a feature vector from a file 
 * @param f File name
 * @return feature vector
 */
fvec_t *fvec_load(char *f)
{
    gzFile z = gzopen(f, "r");
    if (!z) {
        error("Could not open '%s' for reading.", f);
        return NULL;
    }

    fvec_t *fv = fvec_read(z);
    gzclose(z);

    return fv;
}

/**
 * Saves a feature vector to a file 
 * @param fv Feature vector
 * @param f File name
 */
void fvec_save(fvec_t *fv, char *f)
{
    gzFile z = gzopen(f, "w9");
    if (!z) {
        error("Could not open '%s' for writing.", f);
        return;
    }

    fvec_write(fv, z);
    gzclose(z);
}

/**
 * Decodes a string containing delimiters to a global array
 * @param s String containing delimiters
 */
static void decode_delim(const char *s)
{
    char buf[5] = "0x00";
    unsigned int i, j;

    memset(delim, 0, 256);
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
