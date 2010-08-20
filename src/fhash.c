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

/* Hash table */
static fentry_t *fhash = NULL;
static int enabled = FALSE;
static unsigned long collisions = 0;
static unsigned long insertions = 0;

/**
 * Adds a feature and its key to the hash table. The function clones all 
 * input arguments: new memory is allocated and the data is copied.
 * @param k Key for feature
 * @param x Data of feature
 * @param l Length of feature
 */
void fhash_put(feat_t k, char *x, int l)
{
    assert(x && l > 0);
    fentry_t *g, *h;

    if (!enabled)
        return;

    insertions++;
 
    /* Check for duplicate */
    HASH_FIND(hh, fhash, &k, sizeof(feat_t), g);

    /* Check for collision */
    if (g) {
        if (l != g->len || memcmp(x, g->data, l))
            collisions++;
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
    HASH_ADD(hh, fhash, key, sizeof(feat_t), h);
}

/**
 * Gets an entry from the hash table. 
 * @warning The returned memory must not be freed.
 * @param key Feature key
 * @return feature table entry
 */
fentry_t *fhash_get(feat_t key)
{
    fentry_t *f;
    HASH_FIND(hh, fhash, &key, sizeof(feat_t), f);
    return f;
}

/**
 * Creates and allocates the feature hash table.
 */
void fhash_init()
{
    if (fhash)
        fhash_destroy();

    /* Initialize hash fields */
    enabled = TRUE;
    collisions = 0;
    insertions = 0;
}

/**
 * Reset feature hash table.
 */
void fhash_reset()
{
    fhash_init();
}

/**
 * Destroys the feature hash table.
 */
void fhash_destroy()
{
    fentry_t *f;

    while (fhash) {
        f = fhash;
        HASH_DEL(fhash, f);
        free(f->data);
        free(f);
    }

    enabled = FALSE;
    collisions = 0;
    insertions = 0;
}

/**
 * Prints the feature hash table. 
 * @param f File pointer
 * @param fh Feature hash
 */
void fhash_print(FILE *f)
{
    fprintf(f, "Feature hash table [size: %lu, ins: %lu, cols: %lu (%5.2f%%)]\n", 
             fhash_size(), insertions, collisions, 
             (collisions * 100.0) / insertions);
}

/**
 * Returns the size of the feature hash table
 * @param fh Feature hash
 * @return size of table
 */
unsigned long fhash_size()
{
    return HASH_COUNT(fhash);
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
 * @param z File pointer
 */
void fhash_save(gzFile *z)
{
    fentry_t *f;
    int i;

    gzprintf(z, "fhash: len=%u\n", HASH_COUNT(fhash));
    for (f = fhash; f != NULL; f = f->hh.next) {
        gzprintf(z, "  bin=%.16llx: ", (long long unsigned int) f->key);
        for (i = 0; i < f->len; i++) {
            if (f->data[i] != '%' && f->data[i] != ' ' && isprint(f->data[i]))
                gzprintf(z, "%c", f->data[i]);
            else
                gzprintf(z, "%%%.2x", f->data[i]);
        }
        gzprintf(z, "\n");
    }
}

/**
 * Loads the feature hash table from a file stream
 * @param z File pointer
 */
void fhash_load(gzFile *z)
{
    int i, r;
    unsigned long len;
    char buf[512], str[512];
    feat_t key;
    
    fhash_init();
    
    gzgets(z, buf, 512);
    r = sscanf(buf, "fhash: len=%lu\n", (unsigned long *) &len);
    if (r != 1) {
        error("Could not parse feature map");
        return;
    }

    for (i = 0; i < len; i++) {
        gzgets(z, buf, 512);
        r = sscanf(buf, "  bin=%llx:%511s\n", (unsigned long long *) &key,
                   (char *) str);
        if (r != 2) {
            error("Could not parse feature map contents");
            return;
        }

        /* Decode string */
        r = decode_string(str);

        /* Put string to table */
        fhash_put(key, str, r);
    }
}

/**
 * Writes the string of an entry to a file. The string is encoded.
 * @param f File pointer
 * @param fe Feature hash entry
 */
void fhash_print_entry(FILE *f, fentry_t *fe) 
{
    int j;
    
    if (!fe) {
        fprintf(f, "<NULL>");
        return;
    }
    
    for (j = 0; f && j < fe->len; j++) {
        if (fe->data[j] != ' ' && fe->data[j] != '%' && isprint(fe->data[j]))
            fprintf(f, "%c", fe->data[j]);
        else
            fprintf(f, "%%%.2x", (unsigned char) fe->data[j]);
    }
}

/**
 * Returns true if the feature table is enabled
 * @return true if enabled false otherwise
 */
int fhash_enabled()
{
    return enabled;
}

/** @} */
