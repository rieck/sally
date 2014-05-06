/*
 * Sally - A Tool for Embedding Strings in Vector Spaces
 * Copyright (C) 2010-2013 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */

/** 
 * @defgroup sconfig Configuration functions
 * Functions for configuration of the Sally tool. Additionally default
 * values for each configuration parameter are specified in this module.
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "util.h"
#include "sconfig.h"
#include "sally.h"

/* External variables */
extern int verbose;

/* Default configuration */
static config_default_t defaults[] = {
    {"input", "input_format", CONFIG_TYPE_STRING, {.str = "lines"}},
    {"input", "chunk_size", CONFIG_TYPE_INT, {.num = 256}},
    {"input", "decode_str", CONFIG_TYPE_BOOL, {.num = CONFIG_FALSE}},
    {"input", "fasta_regex", CONFIG_TYPE_STRING, {.str = " (\\+|-)?[0-9]+"}},
    {"input", "lines_regex", CONFIG_TYPE_STRING, {.str = "^(\\+|-)?[0-9]+"}},
    {"input", "reverse_str", CONFIG_TYPE_BOOL, {.num = CONFIG_FALSE}},
    {"input", "stopword_file", CONFIG_TYPE_STRING, {.str = ""}},
    {"features", "ngram_len", CONFIG_TYPE_INT, {.num = 4}},
    {"features", "ngram_delim", CONFIG_TYPE_STRING, {.str = "%0a%0d%20"}},
    {"features", "ngram_pos", CONFIG_TYPE_INT, {.num = 0}},
    {"features", "ngram_sort", CONFIG_TYPE_BOOL, {.num = CONFIG_FALSE}},
    {"features", "vect_embed", CONFIG_TYPE_STRING, {.str = "cnt"}},
    {"features", "vect_norm", CONFIG_TYPE_STRING, {.str = "none"}},
    {"features", "vect_sign", CONFIG_TYPE_BOOL, {.num = CONFIG_FALSE}},
    {"features", "thres_low", CONFIG_TYPE_FLOAT, {.flt = 0}},
    {"features", "thres_high", CONFIG_TYPE_FLOAT, {.flt = 0}},    
    {"features", "hash_bits", CONFIG_TYPE_INT, {.num = 22}},
    {"features", "explicit_hash", CONFIG_TYPE_BOOL, {.num = CONFIG_FALSE}},
    {"features", "hash_file", CONFIG_TYPE_STRING, {.str = ""}},    
    {"features", "tfidf_file", CONFIG_TYPE_STRING, {.str = "tfidf.fv"}},
    {"output", "output_format", CONFIG_TYPE_STRING, {.str = "libsvm"}},
    {NULL}
};

/**
 * Print a configuration setting. 
 * @param f File stream to print to
 * @param cs Configuration setting
 * @param d Current depth.
 */
static void config_setting_fprint(FILE *f, config_setting_t * cs, int d)
{
    assert(cs && d >= 0);

    int i;
    for (i = 0; i < d - 1; i++)
        fprintf(f, "       ");

    char *n = config_setting_name(cs);

    switch (config_setting_type(cs)) {
    case CONFIG_TYPE_GROUP:
        if (d > 0)
            fprintf(f, "%s = {\n", n);

        for (i = 0; i < config_setting_length(cs); i++)
            config_setting_fprint(f, config_setting_get_elem(cs, i), d + 1);

        if (d > 0) {
            for (i = 0; i < d - 1; i++)
                fprintf(f, "        ");
            fprintf(f, "};\n\n");
        }
        break;
    case CONFIG_TYPE_STRING:
        fprintf(f, "%s\t= \"%s\";\n", n, config_setting_get_string(cs));
        break;
    case CONFIG_TYPE_FLOAT:
        fprintf(f, "%s\t= %7.5f;\n", n, config_setting_get_float(cs));
        break;
    case CONFIG_TYPE_INT:
        fprintf(f, "%s\t= %d;\n", n, config_setting_get_int(cs));
        break;
    default:
        error("Unsupported type for configuration setting '%s'", n);
        break;
    }
}

/**
 * Print the configuration.
 * @param cfg configuration
 */
void config_print(config_t *cfg)
{
    config_setting_fprint(stdout, config_root_setting(cfg), 0);
}

/**
 * Print the configuration to a file. 
 * @param f pointer to file stream
 * @param cfg configuration
 */
void config_fprint(FILE *f, config_t *cfg)
{
    config_setting_fprint(f, config_root_setting(cfg), 0);
}

/**
 * The functions add default values to unspecified parameters.
 * @param cfg configuration
 */
static void config_default(config_t *cfg)
{
    int i, j;
    const char *s;
    double f;
    config_setting_t *cs, *vs;

    for (i = 0; defaults[i].name; i++) {
        /* Lookup setting group */
        cs = config_lookup(cfg, defaults[i].group);
        if (!cs)
            cs = config_setting_add(config_root_setting(cfg),
                                    defaults[i].group, CONFIG_TYPE_GROUP);

        switch (defaults[i].type) {
        case CONFIG_TYPE_STRING:
            if (config_setting_lookup_string(cs, defaults[i].name, &s))
                continue;

            /* Add default value */
            config_setting_remove(cs, defaults[i].name);
            vs = config_setting_add(cs, defaults[i].name, CONFIG_TYPE_STRING);
            config_setting_set_string(vs, defaults[i].val.str);
            break;
        case CONFIG_TYPE_FLOAT:
            if (config_setting_lookup_float(cs, defaults[i].name, &f))
                continue;

            /* Check for mis-interpreted integer */
            if (config_setting_lookup_int(cs, defaults[i].name, &j)) {
                config_setting_remove(cs, defaults[i].name);
                vs = config_setting_add(cs, defaults[i].name,
                                        CONFIG_TYPE_FLOAT);
                config_setting_set_float(vs, (double) j);
                continue;
            }

            /* Add default value */
            config_setting_remove(cs, defaults[i].name);
            vs = config_setting_add(cs, defaults[i].name, CONFIG_TYPE_FLOAT);
            config_setting_set_float(vs, defaults[i].val.flt);
            break;
        case CONFIG_TYPE_INT:
            if (config_setting_lookup_int(cs, defaults[i].name, &j))
                continue;

            /* Check for mis-interpreted float */
            if (config_setting_lookup_float(cs, defaults[i].name, &f)) {
                config_setting_remove(cs, defaults[i].name);
                vs = config_setting_add(cs, defaults[i].name,
                                        CONFIG_TYPE_INT);
                config_setting_set_int(vs, (long) round(f));
                continue;
            }

            /* Add default value */
            config_setting_remove(cs, defaults[i].name);
            vs = config_setting_add(cs, defaults[i].name, CONFIG_TYPE_INT);
            config_setting_set_int(vs, defaults[i].val.num);
            break;
        case CONFIG_TYPE_BOOL:
            if (config_setting_lookup_bool(cs, defaults[i].name, &j))
                continue;

            /* Check for mis-interpreted integer */
            if (config_setting_lookup_int(cs, defaults[i].name, &j)) {
                config_setting_remove(cs, defaults[i].name);
                vs = config_setting_add(cs, defaults[i].name,
                                        CONFIG_TYPE_BOOL);
                config_setting_set_bool(vs, j == 0 ? CONFIG_FALSE : CONFIG_TRUE);
                continue;
            }

            /* Add default value */
            config_setting_remove(cs, defaults[i].name);
            vs = config_setting_add(cs, defaults[i].name, CONFIG_TYPE_BOOL);
            config_setting_set_bool(vs, defaults[i].val.num);
            break;
        }
    }
}

/**
 * Checks if the configuration is valid and sane. 
 * @return 1 if config is valid, 0 otherwise
 */
int config_check(config_t *cfg)
{
    const char *s1, *s2;
    double f1, f2;
    int i1;

    /* Add default values where missing */
    config_default(cfg);    

    /* Sanity checks */
    config_lookup_string(cfg, "input.stopword_file", &s1);
    config_lookup_string(cfg, "features.vect_delim", &s2);
    if (strlen(s1) > 0 && strlen(s2) == 0) {
        error("Stop words can only be used if delimiters are defined.");
        return 0;
    }
    
    config_lookup_float(cfg, "features.thres_low", &f1);
    config_lookup_float(cfg, "features.thres_high", &f2);
    if (f1 != 0.0 && f2 != 0.0 && f1 > f2) {
        error("Minimum threshold larger than maximum threshold.");
        return 0;
    }

    config_lookup_string(cfg, "features.hash_file", &s1);
    config_lookup_bool(cfg, "features.explicit_hash", &i1);
    if (i1 && strlen(s1) > 0) {
        error("'explicit_hash' and 'hash_file' must not be used togther");
        return 0;
    }
    
    return 1;
}

/** @} */
