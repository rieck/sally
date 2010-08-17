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
 * @defgroup output Output interface
 *
 * Generic implementation of functions for writing vectors in various
 * formats, such as libsvm or matlab format.
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "util.h"
#include "output.h"

/* Modules */
#include "output_libsvm.h"

/**
 * Structure for output interface
 */
typedef struct {
    int (*output_open)(char *);
    int (*output_write)(fvec_t **, int);
    void (*output_close)(void);
} func_t;
static func_t func;

/** 
 * Configure the output of Sally
 * @param format Name of output format
 */
void output_config(char *format)
{
    if (!strcasecmp(format, "libsvm")) {
        func.output_open = output_libsvm_open;
        func.output_write = output_libsvm_write;
        func.output_close = output_libsvm_close;
    } else {
        error("Unknown ouptut format '%s', using 'libsvm' instead.", format);
        output_config("libsvm");
    }
} 

/**
 * Wrapper for opening the output desctination.
 * @param name Name of output destination, e.g., directory or file name
 * @return 1 on success, 0 otherwise.
 */
int output_open(char *name) 
{
    return func.output_open(name);
}

/**
 * Wrapper for writing a block to the output destination.
 * @param x Feature vectors
 * @param len Length of arrays
 * @return Number of written vectors
 */
int output_read(fvec_t **x, int len)
{
    return func.output_write(x, len);
}

/**
 * Wrapper for closing the output destination. 
 */
void output_close(void)
{
    func.output_close();
}

/** @} */
