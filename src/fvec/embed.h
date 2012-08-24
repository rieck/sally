/*
 * Sally - A Tool for Embedding Strings in Vector Spaces
 * Copyright (C) 2010-2012 Konrad Rieck (konrad@mlsec.org);
 *                         Christian Wressnegger(christian@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */

#ifndef EMBED_H
#define EMBED_H

#include "fvec.h"

void fvec_embed(fvec_t *fv, const char *);
void idf_create(char *input);
void idf_destroy();
int idf_check(fvec_t *f);

#endif /* EMBED */
