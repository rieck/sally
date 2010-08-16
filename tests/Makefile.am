# Sally - A Tool for Embedding Strings in Vector Spaces
# Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)

INCLUDES                = @INCLUDES@ -I$(top_srcdir)/src
TESTS                   = test_fhash test_fvec
noinst_PROGRAMS         = test_fhash test_fvec

test_fhash_SOURCES       = test_fhash.c tests.c tests.h
test_fhash_LDADD         = $(top_builddir)/src/libsally.la
test_fhash_LDFLAGS      = -all-static

test_fvec_SOURCES       = test_fvec.c tests.c tests.h
test_fvec_LDADD         = $(top_builddir)/src/libsally.la
test_fvec_LDFLAGS      = -all-static

