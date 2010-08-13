# Sally - A Library for String Features and String Kernels
# Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)

lib_LTLIBRARIES         = libsally.la
include_HEADERS         = sally.h
bin_PROGRAMS		= sally

libsally_la_SOURCES	= sally.c sally.h util.c util.h murmur.c murmur.h \
			  fhash.c fhash.h fvec.c fvec.h md5.c md5.h uthash.h \
			  common.h fmath.c fmath.h export.c export.h

sally_SOURCES		= tool.c tool.h
sally_LDADD             = libsally.la
sally_LDFLAGS           = -all-static
