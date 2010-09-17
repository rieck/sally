# Sally - A Tool for Embedding Strings in Vector Spaces
# Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)

SUBDIRS 		= input output fvec .
INCLUDES                = -I$(top_srcdir)/src/input \
			  -I$(top_srcdir)/src/output \
			  -I$(top_srcdir)/src/fvec

bin_PROGRAMS		= sally
sally_SOURCES		= sally.c sally.h 
sally_LDADD		= libsally.la

noinst_LTLIBRARIES	= libsally.la
libsally_la_SOURCES     = util.c util.h sconfig.c \
			  sconfig.h common.h uthash.h murmur.c \
			  murmur.h md5.c md5.h
libsally_la_LIBADD	= input/libinput.la \
			  output/liboutput.la \
			  fvec/libfvec.la
