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

#ifndef FMAP_H
#define FMAP_H

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

/* Maximum length of stored features */
#define MAX_FEATURE_LEN     64

/** 
 * Entry of feature map
 */
typedef struct {
    feat_t key;            /**< Feature key */
    char *data;            /**< Feature data */
    int len;               /**< Length of data */
    UT_hash_handle hh;     /**< Uthash handle */
} fentry_t;

void fmap_create();
void fmap_destroy();

void fmap_put(feat_t, char *, int);
fentry_t *fmap_get(feat_t);
unsigned long fmap_size();
void fmap_print();
int fmap_enabled();

void fmap_save(FILE *f);
void fmap_load(FILE *f);

#endif                          /* FTABLE_H */
