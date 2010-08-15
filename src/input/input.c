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
 * @defgroup input Input functions
 * Implementation of various functions for reading data.
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
 * Input interface
 */
typedef struct {
    int (*input_open)(char *);
    int (*input_read)(char **, int *, char **, int);
    void (*input_close)(void);
    float (*input_desc2label)(char *);
} func_t;
static func_t func;

/** 
 *
 */
void input_config(input_t in)
{
    switch(in) {
        default:
        case INPUT_DIR:
            func.input_open = input_dir_open;
            func.input_read = input_dir_read;
            func.input_close = input_dir_close;
            func.input_desc2label = input_dir_desc2label;
            break;
#ifdef ENABLE_LIBARCHIVE
        case INPUT_ARC:
            func.input_open = input_arc_open;
            func.input_read = input_arc_read;
            func.input_close = input_arc_close;
            func.input_desc2label = input_arc_desc2label;
            break;
#endif
    }
} 

/**
 *
 */
int input_open(char *name) 
{
    return func.input_open(name);
}

/**
 *
 */
int input_read(char **data, int *sizes, char **desc, int len)
{
    return func.input_read(data, sizes, desc, len);
}

/**
 *
 */
void input_close(void)
{
    func.input_close();
}

/**
 *
 */
float input_desc2label(char *desc)
{
    return func.input_desc2label(desc)
}


/** @} */
