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
 
#ifndef IN_ARC_H
#define IN_ARC_H

#ifdef ENABLE_LIBARCHIVE

int input_arc_open(char *);
int input_arc_read(char **, int *, char **, int);
void input_arc_close();
float input_arc_desc2label(char *);

#endif                          /* ENABLE_LIBARCHIVE */

#endif                          /* IN_ARC_H */
