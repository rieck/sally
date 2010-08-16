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
 * @defgroup sally Sally interface
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
#include "fhash.h"
#include "sally.h"

/* Global verbosity */
int verbose = 0;
config_t cfg;

/**
 * Prints version and copyright information to a file stream
 * @param f File pointer
 * @param p Prefix character
 */
void sally_version(FILE *f, char *p)
{
    fprintf(f, "%s Sally %s - A Tool for Embedding Strings in Vector Spaces\n"
               "%s Copyright (c) 2010 Konrad Rieck (konrad@mlsec.org)\n",
               PACKAGE_VERSION, p, p);
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

/** @} */
