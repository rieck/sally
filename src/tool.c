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

#include "config.h"
#include "common.h"
#include "sally.h"

#include "in_arc.h"

extern int verbose;

/* Static variables */
static sally_t *sa = NULL;

/**
 * Print usage of command line tool
 */  
static void print_usage(void)
{
    printf("Usage: sally [options]\n"
           "Options:\n"
           "  -n <nlen>      Set length of n-grams. Default: %d\n"
           "  -d <delim>     Set delimiter characters. Default: '%s'\n"
           "  -e <embed>     Set embedding mode (cnt, bin). Default: '%s'\n"
           "  -r <norm>      Set normalization mode (l1, l2). Default: '%s'\n"
           "  -b <bits>      Set bits for hashing function. Default: %d\n"
           "  -t             Enable global feature hash table.\n"
           "  -v             Increase verbosity.\n"
           "  -V             Print version and copyright.\n"
           "  -h             Print this help screen.\n", DEFAULT_NLEN, 
           DEFAULT_DELIM, sally_embed2str(DEFAULT_EMBED), 
           sally_norm2str(DEFAULT_NORM), DEFAULT_BITS);
}

/**
 * Print version of Sally tool
 */
static void print_version(void)
{
    sally_version(stdout);
}

/**
 * Parse command line options
 * @param argc Number of arguments
 * @param argv Argument values
 */
static void parse_options(int argc, char **argv)
{
    int ch;
    while ((ch = getopt(argc, argv, "n:e:r:d:b:thvV")) != -1) {
        switch (ch) {
            case 'n':
                sa->nlen = atoi(optarg);
                break;
            case 'e':
                sa->embed = sally_str2embed(optarg);
                break;
            case 'r':
                sa->norm = sally_str2norm(optarg);
                break;
            case 'd':
                sally_set_delim(sa, DEFAULT_DELIM);
                break;
            case 'b':
                sa->bits = atoi(optarg);
                break;
            case 't':
                sally_enable_fhash(sa);
                break;
            case 'v':
                verbose++;
                break;
            case 'V':
                print_version();
                exit(EXIT_SUCCESS);
                break;
            case 'h':
            case '?':
                print_usage();
                exit(EXIT_SUCCESS);
                break;
        }
    }
    
    argc -= optind;
    argv += optind;
    
    if (verbose)
        sally_print(stdout, sa);
        
}

/**
 * Main function of Sally tool 
 * @param argc Number of arguments
 * @param argv Argument values
 * @return exit code
 */
int main(int argc, char **argv)
{
    /* Create Sally */
    sa = sally_create();    

    /* Parse options */
    parse_options(argc, argv);
    char **data = argv + optind; 
    int len = argc - optind;

    #define BLOCK 5

    char *files[BLOCK];
    char *names[BLOCK];
    int sizes[BLOCK];
    
    printf("%s %f\n", data[0], input_arc_desc2label(data[0]));
    
#if 0    
    int num = input_arc_open(data[0]);    
    while (num > 0) {
        int r = input_arc_read(files, sizes, names, BLOCK);
        num -= r;
        
        int i;
        for (i = 0; i < r; i++)
            printf("%s(%d) ", names[i], sizes[i]);
        printf("\n");
    }
    input_arc_close();
#endif
    
    /* Destroy Sally */
    sally_destroy(sa);
}
