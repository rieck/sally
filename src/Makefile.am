# Sally - A Tool for Embedding Strings in Vector Spaces
# Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)

SUBDIRS 		= input output .
INCLUDES                = -I$(top_srcdir)/src/input \
			  -I$(top_srcdir)/src/output 

bin_PROGRAMS		= sally
sally_SOURCES		= sally.c sally.h util.c util.h murmur.c murmur.h \
			  fhash.c fhash.h fvec.c fvec.h md5.c md5.h uthash.h \
			  common.h fmath.c fmath.h sconfig.c sconfig.h
sally_LDADD		= input/libinput.a output/liboutput.a

noinst_LIBRARIES	= libsally.a
libsally_a_SOURCES     = $(sally_SOURCES)
libsally_a_LIBADD	= input/libinput.a output/liboutput.a

