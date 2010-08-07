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
 * @defgroup fmap Global feature map
 * This map keeps track of extracted string features and their 
 * respective hash values.
 *
 * @author Konrad Rieck (konrad.rieck@tu-berlin.de)
 * @{
 */

#include "config.h"
#include "common.h"
#include "fvec.h"
#include "fmap.h"
#include "util.h"

/* Hash table */
static fentry_t *feature_map = NULL;
static int map_enabled = FALSE;
static unsigned long collisions = 0;
static unsigned long insertions = 0;

/* External variables */
extern int verbose;

/**
 * Add a feature and its key to the map. The function clones all input
 * arguments, that is new memory is allocated and the data is copied. 
 * @param k Key for feature
 * @param x Data of feature
 * @param l Length of feature
 */
void fmap_put(feat_t k, char *x, int l)
{
    assert(x && l > 0);
    fentry_t *g, *h;

    if (!map_enabled)
        return;

    /* Check for duplicate */
    HASH_FIND(hh, feature_map, &k, sizeof(feat_t), g);

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
    HASH_ADD(hh, feature_map, key, sizeof(feat_t), h);
    insertions++;
}

/**
 * Gets an entry from the map. The returned memory must not be free'd.
 * @param key Feature key
 * @return feature table entry
 */
fentry_t *fmap_get(feat_t key)
{
    fentry_t *f;
    HASH_FIND(hh, feature_map, &key, sizeof(feat_t), f);
    return f;
}

/**
 * Creates the feature map.
 */
void fmap_create()
{
    if (map_enabled)
        fmap_destroy();

    map_enabled = TRUE;
    collisions = 0;
    insertions = 0;
}

/**
 * Destroy the feature map.
 */
void fmap_destroy()
{
    fentry_t *f;

    while (feature_map) {
        f = feature_map;
        HASH_DEL(feature_map, f);
        free(f->data);
        free(f);
    }

    map_enabled = FALSE;
    collisions = 0;
    insertions = 0;
}

/**
 * Print the feature lookup table. 
 */
void fmap_print()
{
    info_msg(1, "Feature map");
    info_msg(1, "  size: %lu, ins: %lu, colls: %lu (%5.2f%%)", 
             fmap_size(), insertions, collisions, 
             (collisions * 100.0) / insertions);
}

/**
 * Returns the size of the feature map
 * @return size of table
 */
unsigned long fmap_size()
{
    return HASH_COUNT(feature_map);
}

/**
 * Check if feature map is enabled
 */
int fmap_enabled() {
    return map_enabled;
}

/**
 * Decode a string with URI encoding. The function operates 
 * in-place. A trailing NULL character is appended to the string.
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
 * Loads the feature map from a file
 * @param f FILE input stream
 */
void fmap_load(FILE *f)
{
    assert(f);
    int j, r;
    unsigned long len;
    feat_t key;
    char buf[256];

    fmap_create();
    r = fscanf(f, "fm_len %lu\n", &len);

    for (j = 0; j < len; j++) {
    
        r = fscanf(f, "%llx:%255s\n", (unsigned long long *) &key,
                   (char *) buf);
    
        if (r != 2) {
            error("Could not parse feature map");
            return;
        }
        
        r = decode_string(buf);
        fmap_put(key, buf, r);
    }
}


/**
 * Saves the feature map for a given feature vector
 * @param fv Feature vector
 * @param f FILE output stream
 */
void fmap_save(fvec_t *fv, FILE *f)
{
    assert(fv && f);
    fentry_t *fe;
    int i, j;

    fprintf(f, "fm_len %lu\n", fv->len);

    for (j = 0; j < fv->len; j++) {
        fe = fmap_get(fv->dim[j]);
        fprintf(f, "%.16llx:", (long long unsigned int) fe->key);
        for (i = 0; i < fe->len && i < MAX_FEATURE_LEN; i++) {
            if (isprint(fe->data[i]) && fe->data[i] != '%')
                fprintf(f, "%c", fe->data[i]);
            else
                fprintf(f, "%%%.2x", (unsigned char) fe->data[i]);
        }
        fprintf(f, "\n");
    }
}

/** @} */
