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
 
#ifndef OUTPUT_MATLAB_H
#define OUTPUT_MATLAB_H

/* matlab output module */
int output_matlab_open(char *);
int output_matlab_write(fvec_t **, int);
void output_matlab_close(void);

#endif                          /* OUTPUT_MATLAB_H */
