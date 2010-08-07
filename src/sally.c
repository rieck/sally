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
 
/** 
 * @defgroup sally Sally interface.
 * Functions and structures for interfacing with Sally. This file contains
 * the main code for using Sally from within other projects.
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */
        
#include "config.h"
#include "common.h"
#include "sally.h"
#include "util.h"
#include "fvec.h"
#include "fmap.h"
#include "sally.h"

/* Global verbosity */
int verbose = 0;

/**
 * Create a sally structure. The function allocates memory and sets all 
 * learning parameters to default values.
 * @return Initialized sally structure
 */
sally_t *sally_create()
{
    sally_t *j = calloc(1, sizeof(sally_t));
    if (!j) {
        error("Could not allocate sally structure.");
        return NULL;
    }
    
    /* Set default values */
    j->nlen = DEFAULT_NLEN;
    j->norm = DEFAULT_NORM;
    j->bits = DEFAULT_BITS;
    j->fmap = DEFAULT_FMAP;
    
    /* Set delimiters */
    sally_set_delim(j, DEFAULT_DELIM);
        
    return j;
} 

/**
 * Destroys a sally structure. The function frees all memory including 
 * the data structure itself.
 * @param j sally structure
 */ 
void sally_destroy(sally_t *j)
{
    if (!j)
        return;
          
    if (j->fmap)
        fmap_destroy();
        
    free(j);
} 

/**
 * Print version and copyright information
 * @param f File stream
 */
void sally_version(FILE *f)
{
    fprintf(f,
            ".Oo Sally %s - A Library for String Features and String Kernels\n"
            "    Copyright (c) 2010 Konrad Rieck (konrad@mlsec.org)\n",
            PACKAGE_VERSION);
}

/**
 * Returns the string for an normalization mode
 * @param e normalization mode
 * @return String for normalization mode
 */
char *sally_norm2str(norm_t e)
{
    switch(e) {
        case NORM_CNT:
            return "cnt";
        case NORM_BIN:
            return "bin";            
        case NORM_CNT_L1:
            return "cnt_l1";            
        case NORM_CNT_L2:
            return "cnt_l2";            
        case NORM_BIN_L1:
            return "bin_l1";            
        case NORM_BIN_L2:
            return "bin_l2";            
    }
    
    return "unknown";
}

/**
 * Returns the normalization mode for a string
 * @param str String
 * @return normalization mode
 */
norm_t sally_str2norm(char *str)
{
    if (!strcasecmp(str, "cnt"))
        return NORM_CNT;
    if (!strcasecmp(str, "bin"))
        return NORM_BIN;
    if (!strcasecmp(str, "cnt_l1"))
        return NORM_CNT_L1;
    if (!strcasecmp(str, "cnt_l2"))
        return NORM_CNT_L2;
    if (!strcasecmp(str, "bin_l1"))
        return NORM_BIN_L1;
    if (!strcasecmp(str, "bin_l2"))
        return NORM_BIN_L2;
        
    warning("Unknown NORMding mode '%s'. Using 'cnt'.", str);
    return NORM_CNT;
}

/**
 * Decodes a string containing delimiters to global delimiter array
 * @param j sally structure
 * @param s String containing delimiters
 */
void sally_set_delim(sally_t *ja, char *s)
{
    char buf[5] = "0x00";
    unsigned int i, j;

    /* Reset delimiters */
    memset(ja->delim, 0, 256);

    for (i = 0; i < strlen(s); i++) {
        if (s[i] != '%') {
            ja->delim[(unsigned int) s[i]] = 1;
            continue;
        }

        /* Skip truncated sequence */   
        if (strlen(s) - i < 2)
            break;

        buf[2] = s[++i];
        buf[3] = s[++i];
        sscanf(buf, "%x", (unsigned int *) &j);
        ja->delim[j] = 1;
    }
}
 
/** @} */
