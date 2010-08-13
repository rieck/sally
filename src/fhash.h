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
typedef struct {
    feat_t key;            /**< Feature key */
    char *data;            /**< Feature data */
    int len;               /**< Length of data */
    UT_hash_handle hh;     /**< Uthash handle */
} fentry_t;


/**
 * Structure of feature hash
 */
typedef struct {
    fentry_t *hash;         /**< Hash table */
    unsigned long cols;     /**< Collisions */
    unsigned long ins;      /**< Insertions */
} fhash_t;

/* Global functions */
fhash_t *fhash_create();
void fhash_destroy(fhash_t *);
void fhash_put(fhash_t *, feat_t, char *, int);
fentry_t *fhash_get(fhash_t *, feat_t);
unsigned long fhash_size(fhash_t *);
void fhash_print(FILE *, fhash_t *);
void fhash_save(fhash_t *, FILE *f);
fhash_t *fhash_load(FILE *f);

#endif                          /* FHASH_H */
