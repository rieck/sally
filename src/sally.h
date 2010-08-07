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

#ifndef SALLY_H
#define SALLY_H

/* Default n-gram length */
#define DEFAULT_NLEN        2
/* Default normalization mode */
#define DEFAULT_NORM        NORM_BIN_L2
/* Hash bits */
#define DEFAULT_BITS	    26
/* Default delimiters */
#define DEFAULT_DELIM	   "%0a%0d%20"
/* Default setting of feature map */
#define DEFAULT_FMAP        FALSE

/** 
 * NORMding of feature vectors 
 */
typedef enum { 
    NORM_CNT,          /**< Counts of features */
    NORM_BIN,          /**< Binary flags for features */
    NORM_CNT_L1,       /**< Counts of features, L1-normalized  */
    NORM_CNT_L2,       /**< Counts of features, L2-normalized  */
    NORM_BIN_L1,       /**< Binary flags for features, L1-normalized  */
    NORM_BIN_L2        /**< Binary flags for features, L2-normalized  */
} norm_t;

/**
 * Generic configuration of sally
 */
typedef struct {
    int nlen;               /**< Length of n-grams */
    norm_t norm;            /**< Normalization of n-grams */
    int bits;               /**< Bits of hashing */
    char delim[256];        /**< Delimiters */
    int fmap;               /**< Flag for feature map */
} sally_t; 

/* Functions */
sally_t *sally_create(void);
void sally_destroy(sally_t *);

/* Utility functions */
char *sally_norm2str(norm_t e);
norm_t sally_str2norm(char *str);
void sally_version(FILE *f);
void sally_set_delim(sally_t *, char *);

#endif                          /* SALLY_H */
