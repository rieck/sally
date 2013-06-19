/*
 * Sally - A Tool for Embedding Strings in Vector Spaces
 * Copyright (C) 2010-2012 Konrad Rieck (konrad@mlsec.org)
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
#include "sally.h"
#include "input.h"
#include "output.h"
#include "output_scores.h"
#include "fvec.h"
#include "util.h"
#include "sconfig.h"

/* Global variables */
int verbose = 1;
int print_conf = 0;
config_t cfg;

/* Local variables */
static char *input = NULL;
static char *output = NULL;
static long entries = 0;
static long eval_mode = FALSE;

/* Option string */
#define OPTSTRING       "c:i:o:n:d:p:s:E:N:b:vqVhCDw:"

/**
 * Array of options of getopt_long()
 */
static struct option longopts[] = {
    {"config_file", 1, NULL, 'c'},
    {"input_format", 1, NULL, 'i'},
    {"chunk_size", 1, NULL, 1000},
    {"fasta_regex", 1, NULL, 1001},
    {"lines_regex", 1, NULL, 1002},
    {"decode_str", 1, NULL, 1005},
    {"reverse_str", 1, NULL, 1007},
    {"stopword_file", 1, NULL, 1008},
    {"ngram_len", 1, NULL, 'n'},
    {"ngram_delim", 1, NULL, 'd'},
    {"ngram_pos", 1, NULL, 'p'},
    {"ngram_sort", 1, NULL, 's'},
    {"vect_embed", 1, NULL, 'E'},
    {"vect_norm", 1, NULL, 'N'},
    {"vect_sign", 1, NULL, 1006},
    {"thres_low", 1, NULL, 1009},    
    {"thres_high", 1, NULL, 1010},   
    {"hash_bits", 1, NULL, 'b'},
    {"explicit_hash", 1, NULL, 1003},
    {"hash_file", 1, NULL, 1011},   /* <- last entry */   
    {"tfidf_file", 1, NULL, 1004},
    {"output_format", 1, NULL, 'o'},
    {"verbose", 0, NULL, 'v'},
    {"print_config", 0, NULL, 'C'},
    {"print_defaults", 0, NULL, 'D'},
    {"quiet", 0, NULL, 'q'},
    {"version", 0, NULL, 'V'},
    {"help", 0, NULL, 'h'},
    { "weight_vec", 1, NULL, 'w' },
    {NULL, 0, NULL, 0}
};

/**
 * Prints version and copyright information to a file stream
 * @param f File pointer
 * @param p Prefix character
 * @param m Message
 * @return number of written characters
 */
int sally_version(FILE *f, char *p, char *m)
{
    return fprintf(f, "%sSally %s - %s\n", p, PACKAGE_VERSION, m);
}

/**
 * Print configuration
 * @param msg Text to add to output
 */
static void print_config(char *msg)
{
    sally_version(stdout, "# ", msg);
    config_print(&cfg);
}

/**
 * Print usage of command line tool
 */
static void print_usage(void)
{
    printf("Usage: sally [options] <input> <output>\n"
           "\nI/O options:\n"
           "  -i,  --input_format <format>   Set input format for strings.\n"
           "       --chunk_size <num>        Set chunk size for processing.\n"
           "       --decode_str <0|1>        Enable URI-decoding of strings.\n"
           "       --fasta_regex <regex>     Set RE for labels in FASTA data.\n"
           "       --lines_regex <regex>     Set RE for labels in text lines.\n"
           "       --reverse_str <0|1>       Reverse (flip) the input strings.\n"
           "       --stopword_file <file>    Provide a file with stop words.\n"
           "  -o,  --output_format <format>  Set output format for vectors.\n"
           "\nFeature options:\n"
           "  -n,  --ngram_len <num>         Set length of n-grams.\n"
           "  -d,  --ngram_delim <delim>     Set delimiters of words in n-grams.\n"
           "  -p,  --ngram_pos <0|1>         Enable positional n-grams.\n"
           "  -s,  --ngram_sort <0|1>        Enable sorted n-grams (n-perms).\n"
           "  -E,  --vect_embed <embed>      Set embedding mode for vectors.\n"
           "  -N,  --vect_norm <norm>        Set normalization mode for vectors.\n"
           "       --vect_sign <0|1>         Enable signed embedding.\n"
           "       --thres_low <float>       Enable minimum threshold for vectors.\n"
           "       --thres_high <float>      Enable maximum threshold for vectors.\n"
           "  -b,  --hash_bits <num>         Set number of hash bits.\n"
           "       --explicit_hash <0|1>     Enable explicit hash table.\n"
           "       --hash_file <file>        Set file name for explicit hash table.\n"
           "       --tfidf_file <file>       Set file name for TFIDF weighting.\n"
           "\nGeneric options:\n"
           "  -c,  --config_file <file>      Set configuration file.\n"
           "  -v,  --verbose                 Increase verbosity.\n"
           "  -q,  --quiet                   Be quiet during processing.\n"
           "  -C,  --print_config            Print the current configuration.\n"
           "  -D,  --print_defaults          Print the default configuration.\n"
           "  -V,  --version                 Print version and copyright.\n"
           "  -h,  --help                    Print this help screen.\n"
           "\nEvaluation options:\n"
           "  -w,  --weight_vec              The filename of the liblinear weight vector.\n"
           "\n");
}

/**
 * Print version of Sally tool
 */
static void print_version(void)
{
    printf("Sally %s - A Tool for Embedding Strings in Vector Spaces\n"
           "Copyright (c) 2010-2013 Konrad Rieck (konrad@mlsec.org)\n",
           PACKAGE_VERSION);
}

/**
 * Parse command line options
 * @param argc Number of arguments
 * @param argv Argument values
 */
static void sally_parse_options(int argc, char **argv)
{
    int ch, user_conf = FALSE;

    optind = 0;
    int eval_mode = FALSE;

    while ((ch = getopt_long(argc, argv, OPTSTRING, longopts, NULL)) != -1) {
        switch (ch) {
        case 'c':
            /* Skip. See sally_load_config(). */
            user_conf = TRUE;
            break;
        case 'i':
            config_set_string(&cfg, "input.input_format", optarg);
            break;
        case 1000:
            config_set_int(&cfg, "input.chunk_size", atoi(optarg));
            break;
        case 1001:
            config_set_string(&cfg, "input.fasta_regex", optarg);
            break;
        case 1002:
            config_set_string(&cfg, "input.lines_regex", optarg);
            break;
        case 1005:
            config_set_int(&cfg, "input.decode_str", atoi(optarg));
            break;
        case 1006:
            config_set_int(&cfg, "features.vect_sign", atoi(optarg));
            break;
        case 1007:
            config_set_int(&cfg, "input.reverse_str", atoi(optarg));
            break;
        case 1008:
            config_set_string(&cfg, "input.stopword_file", optarg);
            break;
        case 1009:
            config_set_float(&cfg, "features.thres_low", atof(optarg));
            break;
        case 1010:
            config_set_float(&cfg, "features.thres_high", atof(optarg));
            break;
        case 1011:
            config_set_string(&cfg, "features.hash_file", optarg);
            break;
        case 'n':
            config_set_int(&cfg, "features.ngram_len", atoi(optarg));
            break;
        case 'd':
            config_set_string(&cfg, "features.ngram_delim", optarg);
            break;
        case 'p':
            config_set_int(&cfg, "features.ngram_pos", atoi(optarg));
            break;
        case 's':
            config_set_int(&cfg, "features.ngram_sort", atoi(optarg));
            break;
        case 'E':
            config_set_string(&cfg, "features.vect_embed", optarg);
            break;
        case 'N':
            config_set_string(&cfg, "features.vect_norm", optarg);
            break;
        case 'b':
            config_set_int(&cfg, "features.hash_bits", atoi(optarg));
            break;
        case 1003:
            config_set_int(&cfg, "features.explicit_hash", atoi(optarg));
            break;
        case 1004:
            config_set_string(&cfg, "features.tfidf_file", optarg);
            break;
        case 'o':
            config_set_string(&cfg, "output.output_format", optarg);
            break;
        case 'w':
            eval_mode = TRUE;
            config_set_string(&cfg, "eval.weights", optarg);
            break;
        case 'q':
            verbose = 0;
            break;
        case 'v':
            verbose++;
            break;
        case 'D':
            print_config("Default configuration");
            exit(EXIT_SUCCESS);
            break;
        case 'C':
            print_conf = 1;
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

#ifdef ENABLE_EVALTIME
    config_set_int(&cfg, "input.chunk_size", 1);
    verbose = 0;
#endif

    if (eval_mode) {
        const char* const SCORES = "scores";
        info_msg(1, "Evaluation mode: Overriding output_format setting with '%s'", SCORES);
        config_set_string(&cfg, "output.output_format", SCORES);
    }

    /* Check configuration */
    if(!config_check(&cfg)) {
        exit(EXIT_FAILURE);
    }

    /* We are through with parsing. Print the config if requested */
    if (print_conf) {
        print_config("Current configuration");
        exit(EXIT_SUCCESS);
    }

    argc -= optind;
    argv += optind;
    
    /* Check for input and output arguments */
    if (argc != 2) {
        print_usage();
        exit(EXIT_FAILURE);
    } else {
        input = argv[0];
        output = argv[1];
    }
    
    /* Last but not least. Warn about default config */
    if (!user_conf) {
        warning("No config file given. Using defaults (see -D)");
    }
}


/**
 * Load the configuration of Sally
 * @param argc number of arguments
 * @param argv arguments
 */
static void sally_load_config(int argc, char **argv)
{
    char* cfg_file = NULL;
    int ch;

    /* Check for config file in command line */
    while ((ch = getopt_long(argc, argv, OPTSTRING, longopts, NULL)) != -1) {
        switch (ch) {
        case 'c':
            cfg_file = optarg;
            break;
        case '?':
            print_usage();
            exit(EXIT_SUCCESS);
            break;
        default:                                                                    
            /* empty */
            break;
        }
    }

    /* Init and load configuration */
    config_init(&cfg);

    if (cfg_file != NULL) {
        if (config_read_file(&cfg, cfg_file) != CONFIG_TRUE)
            fatal("Could not read configuration (%s in line %d)",
                  config_error_text(&cfg), config_error_line(&cfg));
    }

    /* Check configuration */
    if (!config_check(&cfg)) {
        exit(EXIT_FAILURE);
    }
}


/**
 * Init the Sally tool
 * @param argc number of arguments
 * @param argv arguments
 */
static void sally_init()
{
    int ehash;
    const char *cfg_str;

    if (verbose > 1)
        config_print(&cfg);

    /* Set delimiters */
    config_lookup_string(&cfg, "features.ngram_delim", &cfg_str);
    if (strlen(cfg_str) > 0) 
        fvec_delim_set(cfg_str);

    /* Check for TFIDF weighting */
    config_lookup_string(&cfg, "features.vect_embed", &cfg_str);
    if (!strcasecmp(cfg_str, "tfidf"))
        idf_create(input);

    /* Load stop words */
    config_lookup_string(&cfg, "input.stopword_file", &cfg_str);
    if (strlen(cfg_str) > 0)
        stopwords_load(cfg_str);

    /* Check for feature hash table */
    config_lookup_int(&cfg, "features.explicit_hash", &ehash);
    config_lookup_string(&cfg, "features.hash_file", &cfg_str);
    if (ehash || strlen(cfg_str) > 0) {
        info_msg(1, "Enabling feature hash table.");
        fhash_init();
    }

    /* Open input */
    config_lookup_string(&cfg, "input.input_format", &cfg_str);
    input_config(cfg_str);
    info_msg(1, "Opening '%0.40s' with input module '%s'.", input, cfg_str);
    entries = input_open(input);
    if (entries < 0)
        fatal("Could not open input source");

    /* Open output */
    config_lookup_string(&cfg, "output.output_format", &cfg_str);
    output_config(cfg_str);
    info_msg(1, "Opening '%0.40s' with output module '%s'.", output, cfg_str);
    if (!output_open(output))
        fatal("Could not open output destination");

    eval_mode = (strcmp(cfg_str, "scores") == 0);
}

/**
 * Main processing routine of Sally. This function processes chunks of
 * strings. It might be suitable for OpenMP support in a later version.
 */
static void sally_process()
{
    long read, i, j;
    int chunk;
    const char *hash_file;

	/* Load weight vector */
	fvec_t* w = NULL;

    if (eval_mode)
    {
        const char* cfg_str;

        config_lookup_string(&cfg, "eval.weights", &cfg_str);
        FILE* f = (cfg_str != NULL ? fopen(cfg_str, "r") : NULL);
        if (f == NULL)
        {
            fatal("Cannot open weight vector");
        }

        w = fvec_read_liblinear(f);
        fclose(f);
    }

    /* Check if a hash file is set */
    config_lookup_string(&cfg, "features.hash_file", &hash_file);

    /* Get chunk size */
    config_lookup_int(&cfg, "input.chunk_size", &chunk);

    /* Allocate space */
    fvec_t **fvec = alloca(sizeof(fvec_t *) * chunk);
    string_t *strs = alloca(sizeof(string_t) * chunk);
    double* const scores = (eval_mode ? alloca(sizeof (double) * chunk) : NULL);

    if (!fvec || !strs)
        fatal("Could not allocate memory for embedding");

    info_msg(1, "Processing %d strings in chunks of %d.", entries, chunk);

    double totalTime = 0;
    for (i = 0, read = 0; i < entries; i += read) {
        read = input_read(strs, chunk);
        if (read <= 0)
            fatal("Failed to read strings from input '%s'", input);

        /* Generic preprocessing of input */
        input_preproc(strs, read);

        struct timeval start, end;
        gettimeofday(&start, NULL);

#ifdef ENABLE_OPENMP
#pragma omp parallel for
#endif
        for (j = 0; j < read; j++) {
            fvec[j] = fvec_extract(strs[j].str, strs[j].len);
            fvec_set_label(fvec[j], strs[j].label);
            fvec_set_source(fvec[j], strs[j].src);

            if (eval_mode)
            {
                scores[j] = fvec_dot(fvec[j], w);
            }
        }
        gettimeofday(&end, NULL);
        double diff = TO_SEC(end) -TO_SEC(start);
        totalTime += diff;

        int ret = (eval_mode ?
                    output_scores_write(scores, read):
                    output_write(fvec, read)
                  );

        if (!ret)
        {
            fatal("Failed to write vectors to output '%s'", output);
        }

        /* Free memory */
        input_free(strs, read);
        output_free(fvec, read);

        /* Reset hash if enabled but no hash file is set */
        if (fhash_enabled() && !strlen(hash_file) > 0)
            fhash_reset();

        prog_bar(0, entries, i + read);
    }

    if (eval_mode)
    {
		info_msg(1, "Net calculation time: %.4f seconds", totalTime);
		fvec_destroy(w);
    }
}

/**
 * Exit Sally tool. Close open files and free memory.
 */
static void sally_exit()
{
    int ehash;
    const char *cfg_str, *hash_file;

    info_msg(1, "Flushing. Closing input and output.");
    input_close();
    output_close();

    config_lookup_string(&cfg, "features.vect_embed", &cfg_str);
    if (!strcasecmp(cfg_str, "tfidf"))
        idf_destroy(input);

    config_lookup_string(&cfg, "input.stopword_file", &cfg_str);
    if (strlen(cfg_str) > 0)
        stopwords_destroy();

    config_lookup_string(&cfg, "features.hash_file", &hash_file);
    if (strlen(hash_file) > 0) {
        info_msg(1, "Saving explicit hash table to '%s'.", hash_file);    
        gzFile z = gzopen(hash_file, "w9");
        if (!z)
            error("Could not open hash file '%s'", hash_file);
        fhash_write(z);
        gzclose(z);
    }
    
    config_lookup_int(&cfg, "features.explicit_hash", &ehash);
    if (ehash || strlen(hash_file) > 0)
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
    sally_load_config(argc, argv);
    sally_parse_options(argc, argv);

    sally_init();
    sally_process();
    sally_exit();

    return EXIT_SUCCESS;
}
