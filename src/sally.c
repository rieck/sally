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

/** 
 * @defgroup sally Main interface for Sally.
 *
 * Functions and structures for interfacing with Sally. This file 
 * contains the main code for using Sally from within other projects.
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "sally.h"
#include "util.h"
#include "fvec.h"
#include "fhash.h"
#include "sally.h"

/* Global verbosity */
int verbose = 0;

/**
 * Creates a Sally configuration. The function allocates memory and sets all 
 * parameters to default values.
 * @return Initialized Sally structure
 */
sally_t *sally_create()
{
    sally_t *sa = calloc(1, sizeof(sally_t));
    if (!sa) {
        error("Could not allocate sally structure.");
        return NULL;
    }
    
    /* Set default values */
    sa->nlen = DEFAULT_NLEN;
    sa->norm = DEFAULT_NORM;
    sa->bits = DEFAULT_BITS;
    sa->fhash = DEFAULT_FHASH;
    
    /* Set delimiters */
    sa->delim = malloc(256 * sizeof(char));
    sally_set_delim(sa, DEFAULT_DELIM);
    
    return sa;
} 

/**
 * Destroys a Sally configuration. The function frees all memory including 
 * the data structure itself.
 * @param sa sally structure
 */ 
void sally_destroy(sally_t *sa)
{
    if (!sa)
        return;
    
    if (sa->fhash)
        fhash_destroy(sa->fhash);    
    if (sa->delim)
        free(sa->delim);
    
    free(sa);
} 

/**
 * Prints version and copyright information to a file stream
 * @param f File pointer
 */
void sally_version(FILE *f)
{
    fprintf(f, "# Sally %s - A Tool for Embedding Strings in Vector Spaces\n"
               "# Copyright (c) 2010 Konrad Rieck (konrad@mlsec.org)\n",
               PACKAGE_VERSION);
}

/**
 * Returns the string for the given embedding mode
 * @param e embedding mode
 * @return String for embedding mode
 */
char *sally_embed2str(embed_t e)
{
    switch(e) {
        case EMBED_CNT:
            return "cnt";            
        case EMBED_BIN:
            return "bin";            
    }
    
    return "unknown";
}

/**
 * Returns the embedding mode for the given string
 * @param str String
 * @return embedding mode
 */
norm_t sally_str2embed(char *str)
{
    if (!strcasecmp(str, "cnt"))
        return EMBED_CNT;
    if (!strcasecmp(str, "bin"))
        return EMBED_BIN;
    
    warning("Unknown embedding '%s'. Using 'cnt'.", str);
    return EMBED_CNT;
}

/**
 * Returns the string for the given normalization mode
 * @param n normalization mode
 * @return String for normalization mode
 */
char *sally_norm2str(norm_t n)
{
    switch(n) {
        case NORM_L1:
            return "l1";            
        case NORM_L2:
            return "l2";            
    }
    
    return "unknown";
}

/**
 * Returns the normalization mode for the given string
 * @param str String
 * @return normalization mode
 */
norm_t sally_str2norm(char *str)
{
    if (!strcasecmp(str, "l1"))
        return NORM_L1;
    if (!strcasecmp(str, "l2"))
        return NORM_L2;
    
    warning("Unknown normalization '%s'. Using 'l1'.", str);
    return NORM_L1;
}

/**
 * Decodes a string containing delimiters to global delimiter array
 * @param sa sally structure
 * @param s String containing delimiters
 */
void sally_set_delim(sally_t *sa, char *s)
{
    char buf[5] = "0x00";
    unsigned int i, j;
    
    /* Reset delimiters */
    memset(sa->delim, 0, 256);
    
    for (i = 0; i < strlen(s); i++) {
        if (s[i] != '%') {
            DELIM(sa, s[i]) = 1;
            continue;
        }
        
        /* Skip truncated sequence */   
        if (strlen(s) - i < 2)
            break;
        
        buf[2] = s[++i];
        buf[3] = s[++i];
        sscanf(buf, "%x", (unsigned int *) &j);
        DELIM(sa, j) = 1;
    }
}

/**
 * Prints the configuration of Sally
 * @param f File pointer
 * @param sa Sally configuration
 */ 
void sally_print(FILE *f, sally_t *sa)
{
    assert(sa);
    char str[4 * 256 + 1], *ptr;
    int i = 0;
    
    fprintf(f, "# Sally configuration\n");
    fprintf(f, "#   nlen: %d, bits: %d, embed: %s, norm: %s, fhash: %s\n", 
             sa->nlen, sa->bits, sally_embed2str(sa->embed), 
             sally_norm2str(sa->norm), sa->fhash ? "enabled" : "disabled");
             
    for (i = 0, ptr = str; i < 256; i++) {
        if (DELIM(sa, i)) {
            sprintf(ptr, "%%%.2x ", i);
            ptr += 4; 
        }
    }
    *ptr = 0;
    
    fprintf(f, "#   delim: %s\n", str);
} 

/**
 * Enable feature hash table
 */
void sally_enable_fhash(sally_t *sa) 
{
    if (sa->fhash)
        fhash_destroy(sa->fhash);        
    sa->fhash = fhash_create();
}


/**
 * Enable feature hash table
 */
void sally_disable_fhash(sally_t *sa) 
{
    fhash_destroy(sa->fhash);
    sa->fhash = NULL;
}

/** @} */
