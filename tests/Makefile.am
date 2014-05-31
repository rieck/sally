# Sally - A Tool for Embedding Strings in Vector Spaces
# Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)

AM_CPPFLAGS             = @AM_CPPFLAGS@ -I$(top_srcdir)/src \
			  -I$(top_srcdir)/src/fvec \
			  -I$(top_srcdir)/src/input \
			  -I$(top_srcdir)/src/output
			  
EXTRA_DIST		= test.in
TESTS_ENVIRONMENT       = export TEST_FILE='$(srcdir)/test.in';
TESTS                   = test_fhash test_fvec test_embed test_ngrams
noinst_PROGRAMS         = test_fhash test_fvec test_embed test_ngrams

test_fhash_SOURCES       = test_fhash.c tests.c tests.h
test_fhash_LDADD         = $(top_builddir)/src/libsally.la 

test_fvec_SOURCES       = test_fvec.c tests.c tests.h
test_fvec_LDADD         = $(top_builddir)/src/libsally.la 

test_embed_SOURCES       = test_embed.c tests.c tests.h
test_embed_LDADD         = $(top_builddir)/src/libsally.la 

test_ngrams_SOURCES      = test_ngrams.c tests.c tests.h
test_ngrams_LDADD        = $(top_builddir)/src/libsally.la 


beautify:
	gindent -i4 -npsl -di0 -br -d0 -cli0 -npcs -ce -nfc1 -nut \
		-T string_t -T gzFile -T fvec_t -T fentry_t -T FILE \
		*.c *.h
