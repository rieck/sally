# Sally - A Tool for Embedding Strings in Vector Spaces
# Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)

INCLUDES                = -I$(top_srcdir)/src \
			  -I$(top_srcdir)/src/fvec
			  
EXTRA_DIST		= README

noinst_LTLIBRARIES	= liboutput.la

liboutput_la_SOURCES	= output.c output.h output_libsvm.c output_libsvm.h \
                          output_text.c output_text.h output_matlab.c \
                          output_matlab.h
