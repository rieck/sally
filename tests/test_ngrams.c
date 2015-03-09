/*
 * Sally - A Tool for Embedding Strings in Vector Spaces
 * Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org);
 *                    Christian Wressnegger (christian@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */

#include "tests.h"
#include "sally.h"
#include "fvec.h"
#include "fhash.h"
#include "sconfig.h"

/* Global variables */
int verbose = 5;
config_t cfg;

/* Test structure */
typedef struct
{
    char *str;
    int nlen;
    int flag;
    /* Number of n-grams */
    int len;    
} test_t;

int test_sorted_ngrams()
{
    int i, err = 0;
    fvec_t *f;

    /* Test for sorted n-grams */
    test_t t[] = {
        {"a b c b a", 3, 0, 3},
        {"a b c b a", 3, 1, 2},
        {"a b c b a", 2, 0, 4},
        {"a b c b a", 2, 1, 2},
        {NULL, 0, 0, 0}
    };

    test_printf("Testing sorted n-grams");

    /* Hack to set delimiters */
    config_set_string(&cfg, "features.ngram_gran", "tokens");
    config_set_string(&cfg, "features.ngram_delim", " ");
    fvec_delim_set(" ");     

    for (i = 0; t[i].str; i++) {
        config_set_int(&cfg, "features.ngram_len", t[i].nlen);
        config_set_bool(&cfg, "features.ngram_sort", t[i].flag); 

        /* Extract features */
        f = fvec_extract(t[i].str, strlen(t[i].str));

        /* Check for correct number of dimensions */
        if (f->len != t[i].len) {
            test_error("(%d) len %d != %d", i, f->len, t[i].len);
            err++;
        }

        fvec_destroy(f);
    }

    config_set_bool(&cfg, "features.ngram_sort", 0); 

    test_return(err, i);
    return err;
}

int test_blended_ngrams()
{
    int i, err = 0;
    fvec_t *f;

    /* Test for blended n-grams */
    test_t t[] = {
        {"a b c d e", 3, 0, 3},
        {"a b c d e", 3, 1, 3 + 4 + 5},
        {"a b c d e", 2, 0, 4},
        {"a b c d e", 2, 1, 4 + 5},
        {NULL, 0, 0, 0}
    };

    test_printf("Testing blended n-grams");

    /* Hack to set delimiters */
    config_set_string(&cfg, "features.ngram_gran", "tokens");    
    config_set_string(&cfg, "features.ngram_delim", " ");
    fvec_delim_set(" ");     

    for (i = 0; t[i].str; i++) {
        config_set_int(&cfg, "features.ngram_len", t[i].nlen);
        config_set_bool(&cfg, "features.ngram_blend", t[i].flag); 

        /* Extract features */
        f = fvec_extract(t[i].str, strlen(t[i].str));

        /* Check for correct number of dimensions */
        if (f->len != t[i].len) {
            test_error("(%d) len %d != %d", i, f->len, t[i].len);
            err++;
        }

        fvec_destroy(f);
    }

    config_set_bool(&cfg, "features.ngram_blend", 0); 

    test_return(err, i);
    return err;
}


int test_pos_ngrams()
{
    int i, err = 0;
    fvec_t *f;

    /* Test for positional n-grams */
    test_t t[] = {
        {"b b b b b", 3, 0, 1},
        {"b b b b b", 3, 1, 3},
        {"b b b b b", 2, 0, 1},
        {"b b b b b", 2, 1, 4},
        {NULL, 0, 0, 0}
    };

    test_printf("Testing positional n-grams");

    /* Hack to set delimiters */
    config_set_string(&cfg, "features.ngram_gran", "tokens");    
    config_set_string(&cfg, "features.ngram_delim", " ");
    fvec_delim_set(" ");     

    for (i = 0; t[i].str; i++) {
    
        config_set_int(&cfg, "features.ngram_len", t[i].nlen);
        config_set_bool(&cfg, "features.ngram_pos", t[i].flag);
        config_set_int(&cfg, "features.pos_shift", 0); 

        /* Extract features */
        f = fvec_extract(t[i].str, strlen(t[i].str));

        /* Check for correct number of dimensions */
        if (f->len != t[i].len) {
            test_error("(%d) len %d != %d", i, f->len, t[i].len);
            err++;
        }

        fvec_destroy(f);
    }

    config_set_bool(&cfg, "features.ngram_pos", 0);

    test_return(err, i);
    return err;
}


/**
 * Main function
 */
int main(int argc, char **argv)
{
    int err = FALSE;

    /* Create config */
    config_init(&cfg);
    config_check(&cfg);
    
    fhash_init();

    err |= test_sorted_ngrams();
    err |= test_blended_ngrams();
    err |= test_pos_ngrams();

    fhash_destroy();

    config_destroy(&cfg);
    return err;
}
