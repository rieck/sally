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
 
#ifndef FARRAY_H
#define FARRAY_H

#include <stdio.h>
#include "fvec.h"
#include "sally.h"

/* Allocate memory in blocks of this size */
#define BLOCK_SIZE              (4096 / sizeof(farray_t))

/**
 * Array of feature vectors.
 */
typedef struct {
    fvec_t **x;                 /**< Array of feature vectors */
    unsigned long len;          /**< Length of array */
    char *src;                  /**< Source of array*/
} farray_t;

/* Feature array functions */
farray_t *farray_create(char *);
void farray_add(farray_t *, fvec_t *);
void farray_destroy(farray_t *);
void farray_print(farray_t *);
farray_t *farray_merge(farray_t *, farray_t *);
farray_t *farray_extract(char *, sally_t *);

#endif                          /* FARRAY_H */
