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

#ifndef FHASH_H
#define FHASH_H

#include "fvec.h"

#ifdef HAVE_UTHASH_UTHASH_H
#include <uthash/uthash.h>
#else
#ifdef HAVE_UTHASH_H
#include <uthash.h>
#else
#include "uthash.h"
#endif
#endif

/** 
 * Entry of feature hash
 */
typedef struct
{
    feat_t key;            /**< Feature key */
    char *data;            /**< Feature data */
    int len;               /**< Length of data */
    UT_hash_handle hh;     /**< Uthash handle */
} fentry_t;

/* Global functions */
void fhash_init();
void fhash_reset();
void fhash_destroy();
void fhash_put(feat_t, char *, int);
fentry_t *fhash_get(feat_t);
unsigned long fhash_size();
void fhash_print(FILE *);
void fhash_write(gzFile f);
void fhash_read(gzFile f);
int fhash_enabled();

#endif /* FHASH_H */
