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
 * @defgroup input Input interface
 *
 * Generic implementation of functions for reading strings in various 
 * formats, such as an archive of files or a directory of files.
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "util.h"
#include "input.h"

/* Modules */
#include "input_arc.h"
#include "input_dir.h"

/**
 * Structure for input interface
 */
typedef struct {
    int (*input_open)(char *);
    int (*input_read)(char **, int *, char **, int);
    void (*input_close)(void);
    float (*input_desc2label)(char *);
} func_t;
static func_t func;

/** 
 * Configure the input of Sally
 * @param format Name of input format
 */
void input_config(char *format)
{
    if (!strcasecmp(format, "dir")) {
        func.input_open = input_dir_open;
        func.input_read = input_dir_read;
        func.input_close = input_dir_close;
        func.input_desc2label = input_dir_desc2label;
#ifdef ENABLE_LIBARCHIVE
    } else if (!strcasecmp(format, "arc")) {
        func.input_open = input_arc_open;
        func.input_read = input_arc_read;
        func.input_close = input_arc_close;
        func.input_desc2label = input_arc_desc2label;
#endif
    } else {
        error("Unknown input format '%s'. Selecting 'dir' instead.", format);
        input_config("dir");
    }
} 

/**
 * Wrapper for opening the input source.
 * @param name Name of input source, e.g., directory or file name
 * @return 1 on success, 0 otherwise.
 */
int input_open(char *name) 
{
    return func.input_open(name);
}

/**
 * Wrapper for reading a block from the input source.
 * @param strs Allocated array for data pointers
 * @param sizes Allocated array for data sizes
 * @param desc Allocated array for descriptions
 * @param len Length of allocated arrays
 * @return Number of read sequences
 */
int input_read(char **strs, int *sizes, char **desc, int len)
{
    return func.input_read(strs, sizes, desc, len);
}

/**
 * Wrapper for closing the input source. 
 */
void input_close(void)
{
    func.input_close();
}

/**
 * Wrapper for description to label conversion. The function takes the 
 * description of an input array and returns a label as a float. 
 * @param desc Description as extracted from input_read
 * @return Label as floating point value
 */
float input_desc2label(char *desc)
{
    return func.input_desc2label(desc)
}

/** @} */
