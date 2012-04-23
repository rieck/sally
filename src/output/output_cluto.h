/*
 * Sally - A Tool for Embedding Strings in Vector Spaces
 * Copyright (C) 2011 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */

#ifndef OUTPUT_CLUTO_H
#define OUTPUT_CLUTO_H

/* cluto output module */
int output_cluto_open(char *);
int output_cluto_write(fvec_t **, int);
void output_cluto_close(void);

#endif /* OUTPUT_CLUTO_H */
