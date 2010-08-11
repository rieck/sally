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

#include "config.h"
#include "common.h"
#include "fmap.h"
#include "tests.h"

/* Test file */
#define TEST_FILE               "test.ft"
/* Number of stress runs */
#define STRESS_RUNS             10000
/* String length */
#define STR_LENGTH              1024

/* Global variables */
int verbose = 5;

/* Test structure */
typedef struct {
    feat_t f;
    char *s;
} test_t;

/* Test features */
test_t tests[] = {
    {0, "a b c d e f"},
    {-1, "a b c d e"},
    {1, "a b c d"},
    {0x10, "a b"},
    {0x100, "a"},
    {0xFFF, ""},
    {0, 0}
};

/* 
 * A simple static test for the feature map
 */
int test_static()
{
    int i, j, k, err = 0;
    fentry_t *f;

    test_printf("Creation and maintenance of feature map");

    /* Initialize table */
    fmap_create();
    for (i = 0; tests[i].s != 0; i++)
        fmap_put(tests[i].f, tests[i].s, strlen(tests[i].s) + 1);

    /* Randomly query elements */
    for (j = 0; j < 100; j++) {
        k = rand() % i;
        f = fmap_get(tests[k].f);

        /* Check for correct feature string */
        if (memcmp(f->data, tests[k].s, f->len)) {
            test_error("(%d) '%s' != '%s'", k, f->data, tests[k].s);
            /* ftable_print(); */
            err++;
        }
    }

    /* Destroy table */
    fmap_destroy();

    test_return(err, 100);
    return err;
}

/* 
 * A simple stress test for the feature table
 */
int test_stress()
{
    int i, j, err = 0;
    fentry_t *f;
    feat_t key;
    char buf[STR_LENGTH + 1];

    test_printf("Stress test for feature table");

    /* Initialize table */
    fmap_create();

    for (i = 0; i < STRESS_RUNS; i++) {
        /* Create random key and string */
        key = rand() % 100;
        for (j = 0; j < STR_LENGTH; j++)
            buf[j] = rand() % 10 + '0';
        buf[j] = 0;

        switch (rand() % 2) {
        case 0:
            /* Insert random string */
            fmap_put(key, buf, strlen(buf));
            break;
        case 1:
            /* Query for string */
            f = fmap_get(key);
            break;
        }
    }

    /* Destroy table */
    fmap_destroy();

    test_return(err, STRESS_RUNS);
    return err;
}

/* 
 * A test for loading and saving the feature table
 */
int test_load_save()
{
    int i, j, err = 0;
    FILE *z;
    fentry_t *f;

    test_printf("Loading and saving of feature table");

    /* Initialize table */
    fmap_create();
    for (i = 0; tests[i].s != 0; i++)
        fmap_put(tests[i].f, tests[i].s, strlen(tests[i].s) + 1);

    /* Create and save feature vectors */
    if (!(z = fopen(TEST_FILE, "w"))) {
        printf("Could not create file (ignoring)\n");
        return FALSE;
    }
    fmap_save(z);
    fclose(z);
    fmap_destroy();

    /* Init and load */
    fmap_create();
    z = fopen(TEST_FILE, "r");
    fmap_load(z);
    fclose(z);

    /* Check elements */
    for (j = 0; j < i; j++) {
        f = fmap_get(tests[j].f);

        /* Check for correct feature string */
        if (memcmp(f->data, tests[j].s, f->len)) {
            test_error("(%d) '%s' != '%s'", j, f->data, tests[j].s);
            err++;
        }
    }

    /* Destroy table */
    fmap_destroy();
    unlink(TEST_FILE);

    test_return(err, i);
    return (err > 0);
}


/**
 * Main function
 */
int main(int argc, char **argv)
{
    int err = FALSE;

    err |= test_static();
    err |= test_stress();
    err |= test_load_save();

    return err;
}
