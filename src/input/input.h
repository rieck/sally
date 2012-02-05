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
 
#ifndef INPUT_H
#define INPUT_H

/**
 * Structure for a string
 */
typedef struct {
    char *str;      /* String data (not necessary c-style) */
    int len;        /* Length of string */
    char *src;      /* Optional description of source */
    float label;    /* Optional label of string */
} string_t;

/* Configuration */
void input_config(const char *);
void input_free(string_t *strs, int len);
void input_preproc(string_t *strs, int len);

/* Generic interface */
int input_open(char *);
int input_read(string_t *, int);
void input_close(void);

#endif                          /* INPUT_H */
