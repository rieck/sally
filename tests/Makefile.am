# Sally - A Tool for Embedding Strings in Vector Spaces
# Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)

AM_CPPFLAGS             = @AM_CPPFLAGS@ -I$(top_srcdir)/src \
			  -I$(top_srcdir)/src/fvec \
			  -I$(top_srcdir)/src/input \
			  -I$(top_srcdir)/src/output
			  
EXTRA_DIST		= test.in \
                          test_options.sh \
                          test_options.txt \
                          test_configs.sh \
                          test_configs.txt \
                          config1.cfg \
                          config2.cfg \
                          config3.cfg \
                          strings.txt
                          
TESTS_ENVIRONMENT       = TEST_FILE='$(srcdir)/test.in' \
                          BUILDDIR='$(top_builddir)' \
                          SRCDIR='$(top_srcdir)'
                          
TESTS                   = test_fhash test_fvec test_embed test_ngrams
if !ENABLE_MD5HASH
TESTS                  += test_options.sh test_configs.sh
endif

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
