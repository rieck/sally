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
 * Saves a feature map to a file stream.
 * @param z File pointer
 */
void fmap_save(FILE * z)
{
    fentry_t *f;
    int i;

    fprintf(z, "fmap: len=%u\n", HASH_COUNT(feature_map));
    for (f = feature_map; f != NULL; f = f->hh.next) {
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
 * Loads a feature table from a file stream
 * @param z File pointer
 */
void fmap_load(FILE *z)
{
    int i, r;
    unsigned long len;
    char buf[512], str[512];
    feat_t key;

    fgets(buf, 512, z);
    r = sscanf(buf, "fmap: len=%lu\n", (unsigned long *) &len);
    if (r != 1) {
        error("Could not parse feature map");
        return;
    }

    for (i = 0; i < len; i++) {
        fgets(buf, 512, z);
        r = sscanf(buf, "  %llx:%511s\n", (unsigned long long *) &key,
                   (char *) str);
        if (r != 2) {
            error("Could not parse feature map contents");
            return;
        }

        /* Decode string */
        r = decode_string(str);

        /* Put string to table */
        fmap_put(key, str, r);
    }
}


/** @} */
