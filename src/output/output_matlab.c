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

/** 
 * @addtogroup output 
 * Module 'matlab'.
 * <b>'matlab'</b>: The vectors are exported as a matlab file 
 * version 5. Each vector is stored as a sparse numeric array. 
 * If parameter <code>explicit_hash</code> is enabled in the
 * string features are stored in a separate cell array.
 * 
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h"
#include "util.h"
#include "output.h"
#include "sally.h"
#include "fhash.h"
#include "output_matlab.h"

/* External variables */
extern config_t cfg;

/* Local variables */
static FILE *f = NULL;
static uint32_t bytes = 0;
static uint32_t vectors = 0;

/**
 * Writes a 16-bit integer to a file stream
 * @param i Integer value
 * @param f File pointer
 * @return number of bytes
 */
static int fwrite_uint16(uint16_t i, FILE *f)
{
    return fwrite(&i, sizeof(i), 1, f) * sizeof(i);
}

/**
 * Writes a 32-bit integer to a file stream
 * @param i Integer value
 * @param f File pointer
 * @return number of bytes
 */
static int fwrite_uint32(uint32_t i, FILE *f)
{
    return fwrite(&i, sizeof(i), 1, f) * sizeof(i);
}

/**
 * Writes a float to a file stream
 * @param i float value
 * @param f File pointer
 * @return number of bytes
 */
static int fwrite_float(float i, FILE *f)
{
    return fwrite(&i, sizeof(i), 1, f) * sizeof(i);
}

/**
 * Writes the dimensions of an array to a mat file
 * @param n Flags
 * @param c Class
 * @param z Non-zero elements
 * @param f File pointer
 * @return bytes written
 */
static int fwrite_array_flags(uint8_t n, uint8_t c, uint32_t z, FILE *f)
{
    fwrite_uint32(MAT_TYPE_UINT32, f);
    fwrite_uint32(8, f);
    fwrite_uint32(n << 16 | c, f);
    fwrite_uint32(z, f);

    return 16;
}

/**
 * Writes the dimensions of an array to a mat file
 * @param n Number of dimensions
 * @param m Number of dimensions
 * @param f File pointer
 * @return bytes written
 */
static int fwrite_array_dim(uint32_t n, uint32_t m, FILE *f)
{
    fwrite_uint32(MAT_TYPE_INT32, f);
    fwrite_uint32(8, f);
    fwrite_uint32(n, f);
    fwrite_uint32(m, f);

    return 16;
}
/**
 * Writes the name of an array to a mat file
 * @param n Name of arry
 * @param f File pointer
 * @return bytes written
 */
static int fwrite_array_name(char *n, FILE *f)
{
    int r, l = strlen(n);

    /* Padding for regular format */
    r = l % 8;
    if (r != 0)
        r = 8 - r;

    /* Regular format */
    fwrite_uint32(MAT_TYPE_INT8, f);
    fwrite_uint32(l, f);
    fwrite(n, l, 1, f);
    fseek(f, r, SEEK_CUR);
    return 8 + l + r;
}

/**
 * Writes a feature vector
 * @param fv Feature vector
 * @param f File pointer
 * @return
 */
static int fwrite_fvec_data(fvec_t *fv, FILE *f)
{
    int r = 0, i;
    int bits;
    
    config_lookup_int(&cfg, "features.hash_bits", (int *) &bits);

    fv->len = 2;

    fwrite_uint32(MAT_TYPE_ARRAY, f);
    fwrite_uint32(0, f);
    
    r += fwrite_array_flags(0, MAT_CLASS_SPARSE, fv->len, f);
    r += fwrite_array_dim(1 << bits, 1, f);
    r += fwrite_array_name("fvec", f);

    /* Write row indices */
    r += fwrite_uint32(MAT_TYPE_INT32, f);
    r += fwrite_uint32(fv->len * sizeof(uint32_t), f);
    for (i = 0; i < fv->len; i++) {
        uint32_t dim= fv->dim[i] & 0x7FFFFFFF;
        r += fwrite_uint32(dim, f);
    }
    /* Padding */
    if (fv->len % 2 == 1)
        r += fwrite_uint32(0, f);

    /* Write column indices */
    r += fwrite_uint32(MAT_TYPE_INT32, f);
    r += fwrite_uint32(fv->len * sizeof(uint32_t), f);
    for (i = 0; i < fv->len; i++) {
        r += fwrite_uint32(1, f);
    }
     /* Padding */
    if (fv->len % 2 == 1)
        r += fwrite_uint32(0, f);

    /* Write values */
    r += fwrite_uint32(MAT_TYPE_SINGLE, f);
    r += fwrite_uint32(fv->len * sizeof(float), f);
    for (i = 0; i < fv->len; i++) {
        float val = fv->val[i];
        r += fwrite_float(val, f);
    }
     /* Padding */
    if (fv->len % 2 == 1)
        r += fwrite_uint32(0, f);

    /* Update size */
    fseek(f, - (r + 4), SEEK_CUR);
    fwrite_uint32(r, f);
    fseek(f, + r, SEEK_CUR);

    return r + 8;
}

/**
 * Opens a file for writing matlab format
 * @param fn File name
 * @return number of regular files
 */
int output_matlab_open(char *fn) 
{
    assert(fn);    
    int r = 0, bits;

    config_lookup_int(&cfg, "features.hash_bits", (int *) &bits);
    if (bits > 31) {
        error("Matlab can not handle features with more than 31 bits");
        return FALSE;
    }

    f = fopen(fn, "w");
    if (!f) {
        error("Could not open output file '%s'.", fn);
        return FALSE;
    }
    
    /* Write matlab header */
    r += sally_version(f, "");
    r += fprintf(f, "MAT-file");
    assert(r < 124);

    while (r < 124 && r > 0)
        r += fprintf(f, ".");

    /* Write version stuff */
    r += fwrite_uint16(0x0100, f);
    r += fwrite_uint16(0x4d49, f);

    if (r != 128) {
        error("Could not write header to output file '%s'.", fn);
        return FALSE;
    }

    /* Write tag of cell array */
    fwrite_uint32(MAT_TYPE_ARRAY, f);
    fwrite_uint32(0, f);   
    r = 0;

    /* Here we go. Start a cell array */
    r += fwrite_array_flags(0, MAT_CLASS_CELL, 0, f);
    r += fwrite_array_dim(0, 1, f);
    r += fwrite_array_name("data", f);

    bytes = r;
    vectors = 0;

    return TRUE;
}

/**
 * Writes a block of files to the output
 * @param x Feature vectors
 * @param len Length of block
 * @return number of written files
 */
int output_matlab_write(fvec_t **x, int len)
{
    assert(x && len >= 0);
    int j;

    for (j = 0; j < len; j++) 
        bytes += fwrite_fvec_data(x[j], f);
    vectors += len;
    
    return TRUE;
}

/**
 * Closes an open output file.
 */
void output_matlab_close()
{
    if (!f)
        return;

    /* Fix number of bytes in header */
    fseek(f, 0x84, SEEK_SET);
    fwrite_uint32(bytes, f);

    /* Fix number of vectors in header */
    fseek(f, 0xa0, SEEK_SET);
    fwrite_uint32(vectors, f);

    fclose(f);
}

/** @} */
