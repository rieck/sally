/*
 * Sally - A Tool for Embedding Strings in Vector Spaces
 * Copyright (C) 2010-2012 Konrad Rieck (konrad@mlsec.org);
 *               Christian Wressnegger (christian@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */

/** 
 * @addtogroup input
 * <hr>
 * <em>stdin</em>: The strings are read from standard input (stdin) as text lines. 
 * A label is automatically extracted if the beginning of the line matches a
 * specified regular expression.  @{
 */

#include "config.h"
#include "common.h"
#include "util.h"
#include "murmur.h"
#include "input.h"

#include <regex.h>

/** Static variable */
static regex_t re;
static int line_num = 0;

/** External variables */
extern config_t cfg;


/** 
 * Converts the beginning of a text line to a label. The label is computed 
 * by matching a regular expression, either directly if the match is a 
 * number or indirectly by hashing.
 * @param line Text line
 * @return label value.
 */
static float get_label(char *line)
{
    char *endptr, *name = line, old;
    regmatch_t pmatch[1];

    /* No match found */
    if (regexec(&re, line, 1, pmatch, 0))
        return 0;

    name = line + pmatch[0].rm_so;
    old = line[pmatch[0].rm_eo];
    line[pmatch[0].rm_eo] = 0;

    /* Test direct conversion */
    float f = strtof(name, &endptr);

    /* Compute hash value */
    if (!endptr || strlen(endptr) > 0)
        f = MurmurHash64B(name, strlen(name), 0xc0d3bab3) % 0xffff;

    line[pmatch[0].rm_eo] = old;

    /* Shift string. This is very inefficient. I know */
    memmove(line, line + pmatch[0].rm_eo, strlen(line) - pmatch[0].rm_eo + 1);
    return f;
}

/**
 * Opens stdin for reading
 * @param name File name
 * @return -2 on success ;) and -1 on error
 */
int input_stdin_open(char *name)
{
    assert(name);
    const char *pattern;

    if (stdin == NULL) {
        error("Could not open <stdin>");
        return -1;
    }

    /* Compile regular expression for label */
    config_lookup_string(&cfg, "input.lines_regex", &pattern);
    if (regcomp(&re, pattern, REG_EXTENDED) != 0) {
        error("Could not compile regex for label");
        return -1;
    }

    line_num = 0;
    return -2;
}

/**
 * Reads a block of files into memory.
 * @param strs Array for data
 * @param len Length of block
 * @return number of stdin read into memory
 */
int input_stdin_read(string_t *strs, int len)
{
    assert(strs && len > 0);
    int read, i = 0, j = 0;
    size_t size;
    char buf[32], *line = NULL;

    for (i = 0; i < len; i++) {
        line = NULL;
        read = getline(&line, &size, stdin);
        if (read == -1) {
            free(line);
            break;
        }

        /* Strip newline characters */
        strip_newline(line, read);

        strs[j].label = get_label(line);
        strs[j].str = line;
        strs[j].len = strlen(line);

        snprintf(buf, 32, "line%d", line_num++);
        strs[j].src = strdup(buf);
        j++;
    }

    return j;
}

/**
 * Closes an open directory.
 */
void input_stdin_close()
{
    regfree(&re);
}

/** @} */
