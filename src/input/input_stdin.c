/*
 * Copyright (C) 2012 Alexander Bikadorov (abiku@cs.tu-berlin.de)
 * 
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
 * <em>stdin</em>: A single string is read from standard input.
 * @{
 */

#include "config.h"
#include "common.h"
#include "input.h"
#include "util.h"
#include "stdio.h"

/**
 * Prepare for reading from standard input. 
 * @return number of input or -1 on error
 */
int input_stdin_open(char *notused)
{
    /* TODO: A check whether stdin is really open */
    return 1;
}

/**
 * Reads string from standard input.
 * @param strs Array for data
 * @param lt Length of block
 * @return number of read input
 */
int input_stdin_read(string_t *strs, int lt)
{
    assert(strs && lt > 0);

    char buf[1024];
    size_t len = 1;

    char *x = malloc(sizeof(char) * 1024);
    if (x == NULL) {
        error("Failed to allocate memory");
        return 0;
    }
    x[0] = '\0';

    /* read entire standard input */
    while (fgets(buf, 1024, stdin)) {
        char *old = x;
        len += strlen(buf);
        x = realloc(x, len);
        if (!x) {
            error("Failed to reallocate memory");
            free(old);
            return 0;
        }

        strncat(x, buf, 1023);
    }

    if (ferror(stdin)) {
        free(x);
        error("Error reading from stdin.");
        return 0;
    }

    strs[0].label = 0.0f;
    strs[0].src = strdup("stdin");
    strs[0].str = x;
    strs[0].len = strlen(x);

    return 1;
}

/**
 * Closes standard input.
 */
void input_stdin_close()
{
    fclose(stdin);
}

/** @} */
