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
 * Supported input modes 
 */
typedef enum {
    INPUT_DIR, 		/* Read files from directory */
#ifdef ENABLE_LIBARCHIVE
    INPUT_ARC,		/* Read files from archive */
#endif    
} input_t;

/* Configuration */
void input_config(input_t);

/* Generic interface */
int input_open(char *);
int input_read(char **, int *, char **, int);
void input_close(void);
float input_desc2label(char *);

#endif                          /* INPUT_H */
