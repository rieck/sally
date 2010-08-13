/*
 * Sally - A Library for String Features and String Kernels
 * Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */

/** 
 * @defgroup fhash Feature hash table
 * This hash table keeps track of extracted string features and their 
 * respective hash values. It can be used for explaining but also debugging 
 * extracted feature vectors.
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "fvec.h"
#include "fhash.h"
#include "util.h"

/* External variables */
extern int verbose;

/* Local functions */
static int decode_string(char *str);

/**
 * Adds a feature and its key to the hash table. The function clones all 
 * input arguments: new memory is allocated and the data is copied.
 * @param fh Feature hash 
 * @param k Key for feature
 * @param x Data of feature
 * @param l Length of feature
 */
void fhash_put(fhash_t *fh, feat_t k, char *x, int l)
{
    assert(x && l > 0);
    fentry_t *g, *h;

    fh->ins++;

    /* Check for duplicate */
    HASH_FIND(hh, fh->hash, &k, sizeof(feat_t), g);

    /* Check for collision */
    if (g) {
        if (l != g->len || memcmp(x, g->data, l))
            fh->cols++;
        return;
    }

    /* Build new entry */
    h = malloc(sizeof(fentry_t));
    h->len = l;
    h->key = k;
    h->data = malloc(l);
    if (h->data)
        memcpy(h->data, x, l);
    else
        error("Could not allocate feature data");

    /* Add to hash and count insertion */
    HASH_ADD(hh, fh->hash, key, sizeof(feat_t), h);
}

/**
 * Gets an entry from the hash table. 
 * @warning The returned memory must not be freed.
 * @param fh Feature hash
 * @param key Feature key
 * @return feature table entry
 */
fentry_t *fhash_get(fhash_t *fh, feat_t key)
{
    fentry_t *f;
    HASH_FIND(hh, fh->hash, &key, sizeof(feat_t), f);
    return f;
}

/**
 * Creates and allocates the feature hash table.
 * @return Feature hash
 */
fhash_t *fhash_create()
{
    fhash_t *fh = malloc(sizeof(fhash_t));

    /* Initialize hash fields */
    fh->hash = NULL;
    fh->cols = 0;
    fh->ins = 0;
    return fh;
}

/**
 * Destroys the feature hash table.
 * @param fh Feature hash
 */
void fhash_destroy(fhash_t *fh)
{
    fentry_t *f;

    while (fh->hash) {
        f = fh->hash;
        HASH_DEL(fh->hash, f);
        free(f->data);
        free(f);
    }
    
    free(fh);
}

/**
 * Prints the feature hash table. 
 * @param fh Feature hash
 */
void fhash_print(fhash_t *fh)
{
    printf("# Feature hash table [size: %lu, ins: %lu, cols: %lu (%5.2f%%)]\n", 
             fhash_size(fh), fh->ins, fh->cols, (fh->cols * 100.0) / fh->ins);
}

/**
 * Returns the size of the feature hash table
 * @param fh Feature hash
 * @return size of table
 */
unsigned long fhash_size(fhash_t *fh)
{
    return HASH_COUNT(fh->hash);
}

/**
 * Decodes a string with URI encoding. The function operates 
 * in-place. A trailing NULL character is appended to the string.
 * @private
 * @param str Stirng to escape.
 * @return length of decoded sequence
 */
static int decode_string(char *str)
{
    int j, k, r;
    char hex[5] = "0x00";

    /* Loop over string */
    for (j = 0, k = 0; j < strlen(str); j++, k++) {
        if (str[j] != '%') {
            str[k] = str[j];
        } else {
            /* Check for truncated string */
            if (strlen(str) - j < 2)
                break;

            /* Parse hexadecimal number */
            hex[2] = str[++j];
            hex[3] = str[++j];
            sscanf(hex, "%x", (unsigned int *) &r);
            str[k] = (char) r;
        }
    }

    return k;
}

/**
 * Saves the feature hash table to a file stream.
 * @param fh Feature hash
 * @param z File pointer
 */
void fhash_save(fhash_t *fh, FILE *z)
{
    fentry_t *f;
    int i;

    fprintf(z, "fhash: len=%u\n", HASH_COUNT(fh->hash));
    for (f = fh->hash; f != NULL; f = f->hh.next) {
        fprintf(z, "  %.16llx: ", (long long unsigned int) f->key);
        for (i = 0; i < f->len; i++) {
            if (isprint(f->data[i]) || f->data[i] == '%')
                fprintf(z, "%c", f->data[i]);
            else
                fprintf(z, "%%%.2x", f->data[i]);
        }
        fprintf(z, "\n");
    }
}

/**
 * Loads the feature hash table from a file stream
 * @param z File pointer
 * @return Feature hash
 */
fhash_t *fhash_load(FILE *z)
{
    int i, r;
    unsigned long len;
    char buf[512], str[512];
    feat_t key;
    fhash_t *fh;
    
    fh = calloc(1, sizeof(fhash_t));

    fgets(buf, 512, z);
    r = sscanf(buf, "fhash: len=%lu\n", (unsigned long *) &len);
    if (r != 1) {
        error("Could not parse feature map");
        free(fh);
        return NULL;
    }

    for (i = 0; i < len; i++) {
        fgets(buf, 512, z);
        r = sscanf(buf, "  %llx:%511s\n", (unsigned long long *) &key,
                   (char *) str);
        if (r != 2) {
            error("Could not parse feature map contents");
            free(fh);
            return NULL;
        }

        /* Decode string */
        r = decode_string(str);

        /* Put string to table */
        fhash_put(fh, key, str, r);
    }
    
    return fh;
}

/** @} */
