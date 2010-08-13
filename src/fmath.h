/*
 * Sally - A Library for String Features and String Kernels
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

void fvec_norm(fvec_t *fv, norm_t e);
void fvec_binarize(fvec_t *fv);

#endif                          /* FMATH_H */
