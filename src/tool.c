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

extern int verbose;
exter cfg_t cfg;

/**
 * Print usage of command line tool
 */  
static void print_usage(void)
{
    printf("Usage: sally [options] <config> <input> <output>\n"
           "Options:\n"
           "  -v             Increase verbosity.\n"
           "  -V             Print version and copyright.\n"
           "  -h             Print this help screen.\n");
}

/**
 * Print version of Sally tool
 */
static void print_version(void)
{
    sally_version(stdout, "");
}

/**
 * Parse command line options
 * @param argc Number of arguments
 * @param argv Argument values
 */
static void parse_options(int argc, char **argv)
{
    int ch;
    while ((ch = getopt(argc, argv, "hvV")) != -1) {
        switch (ch) {
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
    /* Parse options */
    parse_options(argc, argv);
}
