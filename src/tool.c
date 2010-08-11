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
#include "sally.h"

extern int verbose;

/* Static variables */
static sally_t *sally = NULL;

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
           "  -m             Enable global feature hash table.\n"
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
    printf("Sally %s - A library for String Features and String Kernels\n"
           "Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)\n", 
           PACKAGE_VERSION);  
}

/**
 * Parse command line options
 * @param argc Number of arguments
 * @param argv Argument values
 */
static void parse_options(int argc, char **argv)
{
    int ch;
    while ((ch = getopt(argc, argv, "n:e:r:d:b:mhvV")) != -1) {
        switch (ch) {
            case 'n':
                sally->nlen = atoi(optarg);
                break;
            case 'e':
                sally->embed = sally_str2embed(optarg);
                break;
            case 'r':
                sally->norm = sally_str2norm(optarg);
                break;
            case 'd':
                sally_set_delim(sally, DEFAULT_DELIM);
                break;
            case 'b':
                sally->bits = atoi(optarg);
                break;
            case 'm':
                sally_enable_fhash(sally);
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
}

/**
 * Main function of Sally tool 
 * @param argc Number of arguments
 * @param argv Argument values
 * @return exit code
 */
int main(int argc, char **argv)
{
    sally = sally_create();

    parse_options(argc, argv);
    
    sally_destroy(sally);
}
