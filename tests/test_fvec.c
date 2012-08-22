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
    char *dlm;
    int nlen;
    int len;
} test_t;

/* Test array of strings */
test_t tests[] = {
    {" a:a a:a a:a a:a ", " ", 1, 1},
    {" a:a a:b a:c a:d ", " ", 1, 4},
    {" a:a b:c a:a b:c ", " :", 1, 3},
    {" a:a a:b a:c a:d ", " :", 1, 4},
    {" a:a a:a a:a a:a ", " ", 2, 1},
    {" a:a a:b a:c a:d ", " ", 2, 3},
    {" a:a a:a a:a a:a ", " :", 2, 1},
    {" a:a a:a a:a a:a ", "", 1, 3},
    {" a:a a:b a:c a:d ", "", 1, 6},
    {" a:a a:a a:a a:a ", "", 2, 4},
    {NULL, NULL, 0}
};

/* Test file */
#define TEST_FILE               "test.fv"
/* Number of stress runs */
#define STRESS_RUNS             1000
/* String length */
#define STR_LENGTH              1024


void init_sally(test_t t)
{
    config_set_string(&cfg, "features.ngram_delim", t.dlm);
    config_set_int(&cfg, "features.ngram_len", t.nlen);
    fvec_delim_set(t.dlm); /* usually done in sally_init */
}


/* 
 * A simple static test for the feature vectors
 */
int test_static()
{
    int i, err = 0;
    fvec_t *f;

    test_printf("Extraction of feature vectors");

    for (i = 0; tests[i].str; i++) {
        init_sally(tests[i]);

        /* Extract features */
        f = fvec_extract(tests[i].str, strlen(tests[i].str));

        /* Check for correct number of dimensions */
        if (f->len != tests[i].len) {
            test_error("(%d) len %d != %d", i, f->len, tests[i].len);
            err++;
        }

        fvec_destroy(f);
    }

    test_return(err, i);
    return err;
}

int test_arithmetic()
{
	int i = 0, err = 0;
	double d1, d2 = 0.0;
    fvec_t *fv1, *fv2, *empty;

	test_printf("Arithmetic operations for feature vectors");

    init_sally(tests[0]);
    fv1 = fvec_extract(tests[0].str, strlen(tests[0].str));
    empty = fvec_extract("", 0);

	/* Multiplication with an empty feature vector */ i++;
    fv2 = fvec_clone(fv1);

    fvec_times(fv2, empty);
    fvec_sparsify(fv2);
    if (fv2->len != 0) {
    	err++;
    	test_error("(%d) len %d != 0", i, fv2->len);
    }

    fvec_destroy(fv2);

	/* Addition with an empty feature vector */ i++;
    fv2 = fvec_clone(fv1);

    fvec_add(fv2, empty);
    if (0 /*fvec_compare(fv1, fv2) */) {
    	err++;
    	test_error("(%d) addition failed!", i);
    }

    fvec_destroy(fv2);

	/* Dot product with an empty feature vector */ i++;
    d1 = fvec_dot(empty, fv1);
    d2 = fvec_dot(fv1, empty);

    if (d1 != 0 || d2 != 0) {
    	err++;
    	test_error("(%d) dot product failed!", i);
    }

	/* Scalar multiplication with 0 */ i++;
    fv2 = fvec_clone(fv1);

    fvec_mul(fv2, 0.0);
    fvec_sparsify(fv2);
    if (fv2->len != 0) {
    	err++;
    	test_error("(%d) scalar product failed!", i);
    }

    fvec_destroy(fv2);

    /* TODO: Test all implemented arithmetic operations for
     * sparse feature vectors
     */

    fvec_destroy(fv1);
    fvec_destroy(empty);

	test_return(err, i);
	return err;
}

/* 
 * A simple stress test for the feature table
 */
int test_stress()
{
    int i, j, err = 0;
    fvec_t *f;
    char buf[STR_LENGTH + 1];

    test_printf("Stress test for feature vectors");
    config_set_string(&cfg, "features.ngram_delim", "0");
    fvec_delim_set("0"); /* usually done in sally_init */
    fhash_init();

    for (i = 0; i < STRESS_RUNS; i++) {
        config_set_int(&cfg, "features.ngram_len", rand() % 10 + 1);

        /* Create random key and string */
        for (j = 0; j < STR_LENGTH; j++)
            buf[j] = rand() % 10 + '0';
        buf[j] = 0;

        /* Extract features */
        f = fvec_extract(buf, strlen(buf));
        /* Destroy features */
        fvec_destroy(f);
    }

    fhash_destroy();
    test_return(err, STRESS_RUNS);
    return err;
}

/* 
 * A simple stress test for the feature table
 */
int test_stress_omp()
{
    int i, j, err = 0;
    fvec_t *f;
    char buf[STR_LENGTH + 1];

    test_printf("Stress test for feature vectors (OpenMP)");
    config_set_string(&cfg, "features.ngram_delim", "0");
    fvec_delim_set("0"); /* usually done in sally_init */
    fhash_init();

#ifdef ENABLE_OPENMP
#pragma omp parallel for
#endif
    for (i = 0; i < STRESS_RUNS; i++) {
        config_set_int(&cfg, "features.ngram_len", rand() % 10 + 1);

        /* Create random key and string */
        for (j = 0; j < STR_LENGTH; j++)
            buf[j] = rand() % 10 + '0';
        buf[j] = 0;

        /* Extract features */
        f = fvec_extract(buf, strlen(buf));

        /* Destroy features */
        fvec_destroy(f);
    }

    fhash_destroy();
    test_return(err, STRESS_RUNS);
    return err;
}


/* 
 * A simple read and write test case
 */
int test_read_write()
{
    int i, j, err = 0;
    fvec_t *f, *g;
    gzFile z;

    test_printf("reading and saving of feature vectors");

    config_set_string(&cfg, "features.ngram_delim", " ");
    config_set_int(&cfg, "features.ngram_len", 2);
    fvec_delim_set(" "); /* usually done in sally_init */

    /* Create and write feature vectors */
    z = gzopen(TEST_FILE, "w9");
    if (!z) {
        printf("Could not create file (ignoring)\n");
        return FALSE;
    }

    for (i = 0; tests[i].str; i++) {
        f = fvec_extract(tests[i].str, strlen(tests[i].str));
        fvec_write(f, z);
        fvec_destroy(f);
    }
    gzclose(z);

    /* read and compare feature vectors */
    z = gzopen(TEST_FILE, "r");

    for (i = 0; tests[i].str; i++) {
        f = fvec_extract(tests[i].str, strlen(tests[i].str));
        g = fvec_read(z);

        /* Check dimensions and values */
        for (j = 0; j < f->len && j < g->len; j++) {
            if (f->dim[j] != g->dim[j]) {
                test_error("(%d) f->dim[%d] != g->dim[%d]", i, j, j);
                break;
            }
            if (fabs(f->val[j] - g->val[j]) > 10e-9) {
                test_error("(%d) f->val[%d] %f != g->val[%d] %f", i, j,
                           f->val[j], j, g->val[j]);
                break;
            }
        }
        err += (j < f->len || j < g->len);

        fvec_destroy(f);
        fvec_destroy(g);
    }

    gzclose(z);
    unlink(TEST_FILE);

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

    err |= test_static();
    err |= test_arithmetic();
    err |= test_stress();
#ifdef ENABLE_OPENMP
    err |= test_stress_omp();
#endif
    err |= test_read_write();

    config_destroy(&cfg);
    return err;
}
