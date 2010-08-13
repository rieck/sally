/*
 * Sally - A Tool for Embedding Strings in a Vector Space
 * Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */

#ifndef IMPORT_H
#define IMPORT_H

#include "sally.h"
#include "fvec.h"

fvec_t **import_fvec_dir(sally_t *sa, char *, int *l);

#endif                          /* IMPORT */
