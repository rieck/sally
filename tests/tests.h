/*
 * Sally - A Tool for Embedding Strings in a Vector Space
 * Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */

#ifndef TESTS_H
#define TESTS_H

#include "config.h"
#include "common.h"
#include "util.h"

/* With of text line */
#define LINE_WIDTH          60

/* Functions */
void test_printf(char *fmt, ...);
void test_return(int, int);
void test_error(char *fmt, ...);

#endif
