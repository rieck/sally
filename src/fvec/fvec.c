/*
 * Sally - A Tool for Embedding Strings in Vector Spaces
 * Copyright (C) 2010-2012 Konrad Rieck (konrad@mlsec.org);
 *                         Christian Wressnegger (christian@mlsec.org)
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
#include "sally.h"
#include "norm.h"
#include "embed.h"

/* External variables */
extern int verbose;
extern config_t cfg;

/* Local functions */
static inline void extract_wgrams(fvec_t *, char *x, int l, int p, int s);
static inline void extract_ngrams(fvec_t *, char *x, int l, int p, int s);
static inline void count_feat(fvec_t *fv);
static inline int cmp_feat(const void *x, const void *y);
static inline void cache_put(fentry_t *c, fvec_t *fv, char *t, int l);
static inline void cache_flush(fentry_t *c, int l);
static inline void fvec_postprocess(fvec_t *fv);
static inline fvec_t *fvec_extract_intern2(char *x, int l);

/* Global delimiter table */
char delim[256] = { DELIM_NOT_INIT };

/**
 * Allocates and extracts a feature vector from a string with
 * postprocessing and blended n-grams.
 * @param x String of bytes (with space delimiters)
 * @param l Length of sequence
 * @return feature vector
 */
fvec_t *fvec_extract(char *x, int l)
{
    /* Extract features */
    fvec_t *fv = fvec_extract_intern(x, l);

    /* Post-processing */
    fvec_postprocess(fv);
    return fv;
}

/*
 * Internal: Allocates and extracts a feature vector from a string
 * without postprocessing but blended n-grams
 * @param x String of bytes (with space delimiters)
 * @param l Length of sequence
 * @return feature vector
 */
fvec_t *fvec_extract_intern(char *x, int l)
{
    int i, blend;
    cfg_int len;

    /* Get config */
    config_lookup_bool(&cfg, "features.ngram_blend", &blend);
    config_lookup_int(&cfg, "features.ngram_len", &len);

    /* Extract n-grams */
    fvec_t *fv = fvec_extract_intern2(x, l);

    /* Blended n-grams */
    for (i = 1; blend && i < len; i++) {
        config_set_int(&cfg, "features.ngram_len", i);
        fvec_t *fx = fvec_extract_intern2(x, l);
        fvec_add(fv, fx);
        fvec_destroy(fx);
    }
    config_set_int(&cfg, "features.ngram_len", len);

    return fv;
}

/**
 * Internal: Allocates and extracts a feature vector from a string without
 * postprocessing and no blended n-grams.
 * @param x String of bytes (with space delimiters)
 * @param l Length of sequence
 * @return feature vector
 */
fvec_t *fvec_extract_intern2(char *x, int l)
{
    fvec_t *fv;
    int pos;
    cfg_int shift;
    const char *dlm_str;
    assert(x && l >= 0);

    /* Allocate feature vector */
    fv = calloc(1, sizeof(fvec_t));
    if (!fv) {
        error("Could not extract feature vector");
        return NULL;
    }

    /* Get configuration */
    config_lookup_string(&cfg, "features.ngram_delim", &dlm_str);
    config_lookup_bool(&cfg, "features.ngram_pos", &pos);
    config_lookup_int(&cfg, "features.pos_shift", &shift);

    /* Check for empty sequence */
    if (l == 0)
        return fv;

    /* Sanitize shift value */
    if (!pos)
        shift = 0;

    /* Allocate arrays */
    int space = 2 * shift + 1;
    fv->dim = (feat_t *) malloc(l * sizeof(feat_t) * space);
    fv->val = (float *) malloc(l * sizeof(float) * space);

    if (!fv->dim || !fv->val) {
        error("Could not allocate feature vector contents");
        fvec_destroy(fv);
        return NULL;
    }

    /* Get configuration */
    config_lookup_string(&cfg, "features.ngram_delim", &dlm_str);

    /* Loop over position shifts (0 if pos is disabled) */
    for (int s = -shift; s <= shift; s++) {
        if (!dlm_str || strlen(dlm_str) == 0) {
            extract_ngrams(fv, x, l, pos, s);
        } else {
            extract_wgrams(fv, x, l, pos, s);
        }
    }

    /* Sort extracted features */
    qsort(fv->dim, fv->len, sizeof(feat_t), cmp_feat);

    /* Count features  */
    count_feat(fv);

    return fv;
}

/**
 * Internal post-processing of feature vectors.
 * @param fv feature vector
 */
void fvec_postprocess(fvec_t *fv)
{
    const char *cfg_str;
    double flt1, flt2;

    /* Compute embedding and normalization */
    config_lookup_string(&cfg, "features.vect_embed", &cfg_str);
    fvec_embed(fv, cfg_str);
    config_lookup_string(&cfg, "features.vect_norm", &cfg_str);
    fvec_norm(fv, cfg_str);

    /* Apply thresholding */
    config_lookup_float(&cfg, "features.thres_low", &flt1);
    config_lookup_float(&cfg, "features.thres_high", &flt2);
    if (flt1 != 0.0 || flt2 != 0.0) {
        fvec_thres(fv, flt1, flt2);
    }
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
 * Truncates the given features vector down to 0-length
 */
void fvec_truncate(fvec_t *const fv)
{
    assert(fv != NULL);

    fv->len = 0;
    if (fv->dim)
        free(fv->dim);
    if (fv->val)
        free(fv->val);

    fv->dim = NULL;
    fv->val = NULL;
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
    c->len = l;
    c->key = fv->dim[fv->len];
    c->data = malloc(l);
    if (c->data)
        memcpy(c->data, t, l);
    else
        error("Could not allocate feature data");
}

/**
 * Flushs all features from the cache to the feature hash table.
 * @param c Pointer to cache
 * @param l Length of cache
 */
static void cache_flush(fentry_t *c, int l)
{
    int i;

    /* Flush cache and add features to hash */
#ifdef HAVE_OPENMP
#pragma omp critical
#endif
    {
        for (i = 0; i < l; i++) {
            fhash_put(c[i].key, c[i].data, c[i].len);
            free(c[i].data);
        }
    }
}

/**
 * Compares two characters (I bet there is a similar function somewhere in 
 * libc and this is just some ugly code).
 * @param v1 first char 
 * @param v2 second char
 * @return comparison result as integer
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
 * @return comparison result as integer
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
 * @param pos Positional n-grams
 * @param shift Shift value
 */
static void extract_wgrams(fvec_t *fv, char *x, int l, int pos, int shift)
{
    assert(fv && x && l > 0);
    int sort, sign, flen;
    cfg_int nlen, bits;
    unsigned int i, j = l, ci = 0;
    unsigned int dlm = 0;
    unsigned int fstart, fnext = 0, fnum = 0;
    char *t = malloc(l + 1), *fstr;
    fentry_t *cache = NULL;

    /* Get configuration */
    config_lookup_int(&cfg, "features.ngram_len", &nlen);
    config_lookup_bool(&cfg, "features.ngram_sort", &sort);
    config_lookup_int(&cfg, "features.hash_bits", &bits);
    config_lookup_bool(&cfg, "features.vect_sign", &sign);

    /* Set bits of hash mask */
    feat_t hash_mask = ((long long unsigned) 2 << (bits - 1)) - 1;

    if (fhash_enabled())
        cache = calloc(l, sizeof(fentry_t));

    /* Find first delimiter symbol */
    for (dlm = 0; !delim[(unsigned char) dlm] && dlm < 256; dlm++);

    /* Remove redundant delimiters */
    for (i = 0, j = 0; i < l; i++) {
        if (delim[(unsigned char) x[i]]) {
            if (j == 0 || delim[(unsigned char) t[j - 1]])
                continue;
            t[j++] = (char) dlm;
        } else {
            t[j++] = x[i];
        }
    }

    /* No characters remaining */
    if (j == 0)
        goto clean;

    /* Add trailing delimiter */
    if (t[j - 1] != dlm)
        t[j++] = (char) dlm;

    /* Extract n-grams */
    for (fstart = i = 0; i < j; i++) {
        /* Count delimiters and remember start position */
        if (t[i] == dlm && ++fnum == 1)
            fnext = i;

        /* Store n-gram */
        if (fnum == nlen && i - fstart > 0) {
            /* Copy feature string and add slack */
            flen = i - fstart;
            fstr = malloc(flen + sizeof(unsigned long));
            memcpy(fstr, t + fstart, flen);

            /* Sorted n-grams code */
            if (sort)
                fstr = sort_words(fstr, flen, dlm);

            /* Positional n-grams code */
            if (pos) {
                int32_t p = ci + shift;
                memcpy(fstr + flen, &p, sizeof(int32_t));
                flen += sizeof(int32_t);
            }

            feat_t h = hash_str(fstr, flen);
            fv->dim[fv->len] = h & hash_mask;
            fv->val[fv->len] = 1;

            /* Signed embedding */
            if (sign)
                fv->val[fv->len] *= (signed) h > 0 ? -1 : 1;

            /* Cache feature and key */
            if (fhash_enabled())
                cache_put(&cache[ci], fv, fstr, flen);

            fstart = fnext + 1, i = fnext, fnum = 0;
            fv->len++;
            ci++;
            free(fstr);
        }
    }

    /* Save extracted n-grams */
    fv->total += fv->len;

  clean:
    if (fhash_enabled()) {
        cache_flush(cache, ci);
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
 * @param pos Positional n-grams 
 * @param shift Shift value
 */
static void extract_ngrams(fvec_t *fv, char *x, int l, int pos, int shift)
{
    assert(fv && x);

    unsigned int i = 0, ci = 0;
    int sort, flen, sign;
    cfg_int nlen, bits;
    char *fstr, *t = x;
    fentry_t *cache = NULL;

    /* Get configuration */
    config_lookup_int(&cfg, "features.ngram_len", &nlen);
    config_lookup_bool(&cfg, "features.ngram_sort", &sort);
    config_lookup_int(&cfg, "features.hash_bits", &bits);
    config_lookup_bool(&cfg, "features.vect_sign", &sign);

    /* Set bits of hash mask */
    feat_t hash_mask = ((long long unsigned) 2 << (bits - 1)) - 1;

    if (fhash_enabled())
        cache = calloc(l, sizeof(fentry_t));

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

        /* Positional n-grams code */
        if (pos) {
            int32_t p = ci + shift;
            memcpy(fstr + flen, &p, sizeof(int32_t));
            flen += sizeof(int32_t);
        }

        feat_t h = hash_str(fstr, flen);
        fv->dim[fv->len] = h & hash_mask;
        fv->val[fv->len] = 1;

        /* Signed embedding */
        if (sign)
            fv->val[fv->len] *= (signed) h > 0 ? -1 : 1;

        /* Cache feature */
        if (fhash_enabled())
            cache_put(&cache[ci], fv, fstr, flen);

        t++;
        fv->len++;
        ci++;
        free(fstr);
    }
    fv->total += fv->len;

    if (!fhash_enabled())
        return;

    /* Flush cache */
    cache_flush(cache, ci);
    free(cache);
}


/**
 * Compares two features values (hashes)
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
 * Counts features in a preliminary feature vector
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
    feat_t *p_dim = NULL;
    float *p_val = NULL;

    if (fv->len <= 0) {
        fvec_truncate(fv);
        return;
    }

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
                 (unsigned long long) f->dim[i], (double) f->val[i]);
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
 * Decodes a string containing delimiters to a lookup table
 * @param s String containing delimiters
 */
void fvec_delim_set(const char *s)
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
void fvec_delim_reset()
{
    delim[0] = DELIM_NOT_INIT;
}

/** @} */
