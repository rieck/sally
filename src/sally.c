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
#include "input.h"
#include "output.h"
#include "fvec.h"
#include "util.h"
#include "sconfig.h"
#include "fhash.h"

/* Global variables */
int verbose;
config_t cfg;

/* Local variables */
static char *cfg_file = NULL;
static char *input = NULL;
static char *output = NULL;
static long entries = 0;

/**
 * Prints version and copyright information to a file stream
 * @param f File pointer
 * @param p Prefix character
 */
void sally_version(FILE *f, char *p)
{
    fprintf(f, "%s Sally %s - A Tool for Embedding Strings in Vector Spaces\n"
            "%s Copyright (c) 2010 Konrad Rieck (konrad@mlsec.org)\n",
            p, PACKAGE_VERSION, p);
}

/**
 * Main processing function
 * @param in Input file
 * @param out Output file
 */

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
    
    /* Check remaining arguments */
    if (argc != 3) {
        print_usage();
        exit(EXIT_FAILURE);    
    }
    
    cfg_file = argv[0];
    input = argv[1];
    output = argv[2];
}

static void sally_init(int argc, char **argv)
{
    long ehash;
    const char *cfg_str;
    
    /* Parse options */
    parse_options(argc, argv);
    
    /* Init and load configuration */
    config_init(&cfg);
    if (config_read_file(&cfg, cfg_file) != CONFIG_TRUE)
        fatal("Could not read configuration (%s in line %d)",
              config_error_text(&cfg), config_error_line(&cfg));
    
    /* Check configuration */
    config_check(&cfg);
    if (verbose > 1)
        config_print(&cfg);
    
    /* Check for feature hash table */
    config_lookup_int(&cfg, "features.explicit_hash", &ehash);
    if (ehash)
        fhash_init();
    
    /* Open input */
    config_lookup_string(&cfg, "input.format", &cfg_str);
    input_config(cfg_str);
    info_msg(1, "Opening input'%s' [format: %s]", input, cfg_str);
    entries = input_open(input);
    if (entries < 0)
        fatal("Could not open input source");

    config_lookup_string(&cfg, "output.format", &cfg_str);
    output_config(cfg_str);    
    info_msg(1, "Opening output'%s' [format: %s]", output, cfg_str);
    if (!output_open(output))
        fatal("Coult not open output destination");
}

static void sally_process()
{
    long read, i = 0, j, block;
    
    /* Get block size */
    config_lookup_int(&cfg, "input.block_size", &block);
    
    fvec_t **fvec = malloc(sizeof(fvec_t *) * block);
    string_t *strs = malloc(sizeof(string_t) * block);
    
    if (!fvec || !strs) 
        fatal("Could not allocate memory for embedding");
    
    while (i < entries) {
        read = input_read(strs, block);
        if (!read) 
            fatal("Failed to read strings from input '%s'", input);

        
        for (j = 0; j < read; j++) {
            fvec[j] = fvec_extract(strs[j].str, strs[j].len);
            fvec_set_label(fvec[j], strs[j].label);
            fvec_set_source(fvec[j], strs[j].src);
        }

        if (!output_write(fvec, read))
            fatal("Failed to write vectors to output '%s'", output);
        
        for (j = 0; j < read; j++) {
            if (strs[j].src)
                free(strs[j].src);
            if (strs[j].str)
                free(strs[j].str);
            fvec_destroy(fvec[j]);
        }
        
        if (fhash_enabled())
            fhash_reset();
        i += read;
    }
    
    free(strs);
    free(fvec);
}

static void sally_exit()
{
    long ehash;
    
    /* Close input and output */
    input_close();
    output_close();
    
    /* Check for feature hash table */
    config_lookup_int(&cfg, "features.explicit_hash", &ehash);
    if (ehash)
        fhash_destroy();
    
    /* Destroy configuration */
    config_destroy(&cfg);
    
}

/**
 * Main function of Sally tool 
 * @param argc Number of arguments
 * @param argv Argument values
 * @return exit code
 */
int main(int argc, char **argv)
{
    
    sally_init(argc, argv);
    sally_process();
    sally_exit();
}
