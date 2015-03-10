/*
 * Sally - A Tool for Embedding Strings in Vector Spaces
 * Copyright (C) 2010-2012 Konrad Rieck (konrad@mlsec.org);
 *                         Christian Wressnegger (christian@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */

#include "config.h"
#include "common.h"
#include "fhash.h"
#include "tests.h"
#include "input.h"
#include "sconfig.h"
#include "embed.h"

/* Test file */
#define TEST_TFIDF              "test.fv"

/* Global variables */
int verbose = 0;
config_t cfg;

/* 
 * A simple test for the binary embedding
 */
int test_embed_tfidf()
{
    int i, j, n, err = 0;
    string_t strs[10];

    config_set_string(&cfg, "features.vect_norm", "none");
    config_set_string(&cfg, "features.tfidf_file", TEST_TFIDF);

    unlink(TEST_TFIDF);
    char *test_file = getenv("TEST_FILE");
    idf_create(test_file);
    test_printf("Testing TFIDF embedding");

    input_config("lines");
    n = input_open(test_file);
    input_read(strs, n);

    /* Compute IDF manually */
    config_set_string(&cfg, "features.vect_embed", "bin");
    fvec_t *w = fvec_zero();
    for (i = 0, err = 0; i < n; i++) {
        fvec_t *fv = fvec_extract(strs[i].str, strs[i].len);
        fvec_add(w, fv);
        fvec_destroy(fv);
    }
    fvec_invert(w);
    fvec_mul(w, n);
    fvec_log2(w);

    if (!idf_check(w)) {
        err++;
        test_error("(%d) internal idf values seem to be wrong", i);
    }

    /* Invert w for multiplying out IDFs */
    fvec_invert(w);

    config_set_string(&cfg, "features.vect_embed", "tfidf");
    for (i = 0, err = 0; i < n; i++) {
        fvec_t *fv = fvec_extract(strs[i].str, strs[i].len);
        fvec_times(fv, w);

        /* Check if rest tf */
        double d = 0;
        for (j = 0; j < fv->len; j++)
            d += fv->val[j];
        err += fabs(d - 1.0) > 1e-6;
        fvec_destroy(fv);
    }
    test_return(err, n);

    fvec_destroy(w);
    input_free(strs, n);
    input_close();

    idf_destroy();
    unlink(TEST_TFIDF);

    return err;
}

/* 
 * A simple test for the binary embedding
 */
int test_embed_bin()
{
    int i, j, n, err = 0;
    string_t strs[10];

    input_config("lines");
    char *test_file = getenv("TEST_FILE");
    n = input_open(test_file);
    input_read(strs, n);

    test_printf("Testing binary embedding");
    config_set_string(&cfg, "features.vect_embed", "bin");
    config_set_string(&cfg, "features.vect_norm", "none");

    for (i = 0, err = 0; i < n; i++) {
        fvec_t *fv = fvec_extract(strs[i].str, strs[i].len);
        double n = 0;
        for (j = 0; j < fv->len; j++)
            n += fv->val[j];
        err += fabs(n - fv->len) > 1e-6;
        fvec_destroy(fv);
    }
    test_return(err, n);

    input_free(strs, n);
    input_close();
    return err;
}

/* 
 * A simple test for the l2 norm
 */
int test_norm_l2()
{
    int i, j, n, err = 0;
    string_t strs[10];

    input_config("lines");
    char *test_file = getenv("TEST_FILE");
    n = input_open(test_file);
    input_read(strs, n);

    test_printf("Testing L2 normalization");
    config_set_string(&cfg, "features.vect_norm", "l2");
    for (i = 0, err = 0; i < n; i++) {
        fvec_t *fv = fvec_extract(strs[i].str, strs[i].len);
        double n = 0;
        for (j = 0; j < fv->len; j++)
            n += fv->val[j] * fv->val[j];
        err += fabs(sqrt(n) - 1.0) > 1e-6;
        fvec_destroy(fv);
    }
    test_return(err, n);

    input_free(strs, n);
    input_close();
    return err;
}

/* 
 * A simple test for l1 norm
 */
int test_norm_l1()
{
    int i, j, n, err = 0;
    string_t strs[10];

    input_config("lines");
    char *test_file = getenv("TEST_FILE");
    n = input_open(test_file);
    input_read(strs, n);

    test_printf("Testing L1 normalization");
    config_set_string(&cfg, "features.vect_norm", "l1");
    for (i = 0, err = 0; i < n; i++) {
        fvec_t *fv = fvec_extract(strs[i].str, strs[i].len);
        double n = 0;
        for (j = 0; j < fv->len; j++)
            n += fv->val[j];
        err += fabs(n - 1.0) > 1e-6;
        fvec_destroy(fv);
    }
    test_return(err, n);

    input_free(strs, n);
    input_close();
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

    config_set_string(&cfg, "features.granularity", "tokens");
    config_set_string(&cfg, "features.token_delim", " .,%0a%0d");
    config_set_int(&cfg, "features.ngram_len", 1);
    config_set_string(&cfg, "input.input_format", "lines");

    err |= test_norm_l1();
    err |= test_norm_l2();
    err |= test_embed_tfidf();
    err |= test_embed_bin();

    return err;
}
