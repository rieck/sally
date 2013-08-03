# Sally - A Tool for Embedding Strings in Vector Spaces
# Copyright (C) 2010-2011 Konrad Rieck (konrad@mlsec.org)

AM_CPPFLAGS             = -I$(top_srcdir)/src \
			  -I$(top_srcdir)/src/fvec
			  
EXTRA_DIST		= README.md

noinst_LTLIBRARIES	= liboutput.la

liboutput_la_SOURCES	= output.c output.h output_libsvm.c output_libsvm.h \
                          output_text.c output_text.h output_matlab.c \
                          output_matlab.h output_cluto.c output_cluto.h

beautify:
	gindent -i4 -npsl -di0 -br -d0 -cli0 -npcs -ce -nfc1 -nut \
		-T string_t -T gzFile -T fvec_t -T FILE \
		$(liboutput_la_SOURCES)                          
