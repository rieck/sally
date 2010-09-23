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
 * @defgroup util Utility functions
 * The module contains utility functions for Sally.
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
/** Progress bar (with NULL) */
static char pb_string[PROGBAR_LEN + 1];
/** Start timestamp measured */
static double pb_start = -1;

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
    
    fprintf(stderr, "> %s\n", s);
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

/**
 * Print a progress bar in a given range.
 * @param a Minimum value 
 * @param b Maximum value
 * @param c Current value
 */
void prog_bar(long a, long b, long c)
{
    int i, first, last;
    double perc, ptime = 0, min, max, in;
    char *descr = "";
    
    if (verbose == 0)
        return;
    
    min = (double) a;
    max = (double) b;
    in = (double) c;

    perc = (in - min) / (max - min);
    first = fabs(in - min) < 1e-10;
    last = fabs(in - max) < 1e-10;

    /* Start of progress */
    if (pb_start < 0 || (first && !last)) {
        pb_start = time_stamp();
        for (i = 0; i < PROGBAR_LEN; i++)
            pb_string[i] = PROGBAR_EMPTY;
        descr = "start";
        perc = 0.0;
    }

    /* End of progress */
    if (last) {
        for (i = 0; i < PROGBAR_LEN; i++)
            pb_string[i] = PROGBAR_FULL;
        ptime = time_stamp() - pb_start;
        descr = "total";
        perc = 1.0;
        pb_start = -1;
    }

    /* Middle of progress */
    if (!first && !last) {
        int len = (int) round(perc * PROGBAR_LEN);
        for (i = 0; i < len; i++)
            if (i < len - 1)
                pb_string[i] = PROGBAR_DONE;
            else
                pb_string[i] = PROGBAR_FRONT;
        ptime = (max - in) * (time_stamp() - pb_start) / (in - min);
        descr = "   in";
    }

    int mins = (int) floor(ptime / 60);
    int secs = (int) floor(ptime - mins * 60);
    pb_string[PROGBAR_LEN] = 0;

    printf("\r  [%s] %5.1f%%  %s %.2dm %.2ds ", pb_string,
           perc * 100, descr, mins, secs);

    if (last)
        printf("\n");

    fflush(stdout);
    fflush(stderr);
}

#define BLOCK_SIZE 4096

/**
 * Dirty re-write of the GNU getline() function. I have been
 * searching the Web for a couple of minutes to find a suitable 
 * implementation. Unfortunately, I could not find anything 
 * appropriate. Some people confused fgets() with getline(), 
 * others were arguing on licences over and over.
 */
size_t gzgetline(char **s, size_t *n, gzFile *f)
{
      assert(f);
      int c = 0;
      *n = 0;      
 
      if (gzeof(f)) 
          return -1;
      
      while (c != '\n') { 
          if (!*s || *n % BLOCK_SIZE == 0) {
              *s = realloc(*s, *n + BLOCK_SIZE + 1);
              if (!*s)
                  return -1;
          }   
          
          c = gzgetc(f);   
          if (c == -1) 
              break;
          
          (*s)[(*n)++] = c;
      }
      
      (*s)[*n] = 0;
      return *n;
}

/** 
 * Another dirty function to trim strings from leading and trailing 
 * blanks. The original string is modified in place.
 * @param x Input string
 */
void strtrim(char *x) 
{
    assert(x);
    int i = 0, j = 0, l = strlen(x);

    if (l == 0)
        return;

    for (i = 0; i <  l; i++) 
        if (!isspace(x[i]))
            break;

    for (j = l; j > 0; j--) 
        if (!isspace(x[j - 1]))
            break;

    if (j > i) {
        memmove(x, x + i, j - i);
        x[j - i] = 0;
    } else {
        x[0] = 0;
    }
}

/** @} */
  