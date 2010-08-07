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
 
/**
 * @defgroup util Utility functions
 * The module contains utility functions.
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "config.h"
#include "common.h" 
#include "util.h"

/* External variable */
extern int verbose;
/* Global variable */
static double time_start = -1;

/**
 * Loads a textual file into a string. The string is allocated 
 * and need to be free'd later by the caller.
 * @param path Path to file
 * @param name File name or NULL
 * @return string data
 */
char *load_file(char *path, char *name)
{
    assert(path);
    long len, read, i;
    char *x = NULL, file[512];
    struct stat st;

#pragma omp critical (snprintf)
    {
        /* snprintf is not necessary thread-safe. good to know. */
        if (name)
            snprintf(file, 512, "%s/%s", path, name);
        else
            snprintf(file, 512, "%s", path);
    }

    /* Open file */
    FILE *fptr = fopen(file, "r");
    if (!fptr) {
        error("Could not open file '%s'", file);
        return NULL;
    }

    /* Allocate memory */
    stat(file, &st);
    len = st.st_size;
    if (!(x = malloc((len + 1) * sizeof(char)))) {
        error("Could not allocate memory for file data");
        return NULL;
    }

    /* Read data */
    read = fread(x, sizeof(char), len, fptr);
    fclose(fptr);
    if (len != read)
        warning("Could not read all data from file '%s'", file);

    /* Replace null bytes by space */
    for (i = 0; i < len; i++) 
        if (x[i] == 0)
            x[i] = ' ';

    /* Terminate string */
    x[len] = '\0';
    return x;
}

/**
 * Returns the number of entries in a directory. 
 * @param dir Directory to analyse
 * @param fnum Return pointer for number of regular files
 * @param total Return pointer for number of total files
 */
void list_dentries(char *dir, int *fnum, int *total)
{  
    struct dirent *dp;
    DIR *d;

    *fnum = 0;
    *total = 0;

    d = opendir(dir);
    while (d && (dp = readdir(d)) != NULL) {
#ifdef __MINGW32__  
            ++*fnum;
#else
        if (dp->d_type == DT_REG)
            ++*fnum;
#endif            
        ++*total;
    }
    closedir(d);
}

/**
 * Extracts the suffix from a file name. If the file does not
 * have a suffix, the function returns "unknown". 
 */
char *file_suffix(char *file)
{
    char *name = file + strlen(file) - 1;

    /* Determine dot in file name */
    while (name != file && *name != '.')
        name--; 

    /* Check for files with no suffix */
    if (name == file)
        name = "unknown";
    else
        name++;

    return name;
}

/**
 * Print a formated info message with timestamp. 
 * @param v Verbosity level of message
 * @param m Format string
 */
void info_msg(int v, char *m, ...)
{
    va_list ap;
    char s[256] = { " " };

    if (time_start == -1)
        time_start = time_stamp();

    if (v > verbose)
        return;

    va_start(ap, m);
    vsnprintf(s, 256, m, ap);
    va_end(ap);
    
    fprintf(stderr, "[%7.1f] %s\n", time_stamp() - time_start, s);
    fflush(stderr);
}


/**
 * Print a formated error/warning message. See the macros error and 
 * warning in util.h
 * @param p Prefix string, e.g. "Error"
 * @param f Function name
 * @param m Format string
 */
void err_msg(char *p, const char *f, char *m, ...) 
{
    va_list ap;
    char s[256] = { " " };
    
    va_start(ap, m);
    vsnprintf(s, 256, m, ap);
    va_end(ap);

    fprintf(stderr, "%s: %s (", p, s);
    if (errno)
        fprintf(stderr, "%s, ", strerror(errno));
    fprintf(stderr, "%s)\n", f);
    errno = 0;
}


/**
 * Return a timestamp of the real time
 * @return time stamp
 */
double time_stamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

/** @} */
