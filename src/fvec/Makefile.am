# Sally - A Tool for Embedding Strings in Vector Spaces
# Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)

AM_CPPFLAGS             = -I$(top_srcdir)/src \
			  -I$(top_srcdir)/src/input

noinst_LTLIBRARIES	= libfvec.la

libfvec_la_SOURCES	= fhash.c fhash.h fvec.c fvec.h \
			  fmath.c fmath.h embed.c embed.h \
			  norm.c norm.h reduce.c reduce.h

beautify:
	gindent -i4 -npsl -di0 -br -d0 -cli0 -npcs -ce -nfc1 -nut \
		-T fvec_t -T string_t -T gzFile -T feat_t \
		-T fentry_t \
		$(libfvec_la_SOURCES)
	