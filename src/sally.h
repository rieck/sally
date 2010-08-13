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
/* Default embedding mode */
#define DEFAULT_EMBED       EMBED_CNT
/* Default normalization mode */
#define DEFAULT_NORM        NORM_L2
/* Default bits of feature hash */
#define DEFAULT_BITS	    26
/* Default delimiters */
#define DEFAULT_DELIM	   "%0a%0d%20"
/* Default setting of feature hash */
#define DEFAULT_FHASH       NULL

/* Convenience macro */
#define DELIM(s, i)    ((char *) s->delim)[(unsigned int) i]

/** 
 * Normalization modes
 */
typedef enum { 
    NORM_L1,                /**< L1 Norm */
    NORM_L2,                /**< L2 Norm */
} norm_t;

/** 
 * Embedding modes
 */
typedef enum { 
    EMBED_CNT,              /**< Counts of features */
    EMBED_BIN,              /**< Binary flags for features */
} embed_t;

/**
 * Generic configuration of Sally
 */
typedef struct {
    int nlen;               /**< Length of n-grams */
    embed_t embed;          /**< Embedding mode */
    norm_t norm;            /**< Normalization of vectors */
    int bits;               /**< Bits of hashing */
    void *delim;            /**< Delimiters */
    void *fhash;            /**< Feature hash table */
} sally_t; 

/* Functions */
sally_t *sally_create(void);
void sally_destroy(sally_t *);
void sally_version(FILE *f);
void sally_print(FILE *f, sally_t *);

/* Configuration functions */
void sally_set_delim(sally_t *, char *);
void sally_enable_fhash(sally_t *sa);
void sally_disable_fhash(sally_t *sa);
char *sally_norm2str(norm_t e);
norm_t sally_str2norm(char *str);
char *sally_embed2str(embed_t e);
norm_t sally_str2embed(char *str);

#endif                          /* SALLY_H */
