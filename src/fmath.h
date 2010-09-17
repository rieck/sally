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

#ifndef FMATH_H
#define FMATH_H

#include "fvec.h"

void fvec_norm(fvec_t *fv, const char *);
void fvec_embed(fvec_t *fv, const char *);
void fvec_binarize(fvec_t *fv);
fvec_t *fvec_clone(fvec_t *);
double fvec_dot(fvec_t *fa, fvec_t *fb);
void fvec_add(fvec_t *fa, fvec_t *fb);
void fvec_times(fvec_t *fa, fvec_t *fb);
void fvec_mul(fvec_t *f, double s);

#endif                          /* FMATH_H */
