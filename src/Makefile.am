# Sally - A Library for String Features and String Kernels
# Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)

lib_LTLIBRARIES         = libsally.la
include_HEADERS         = sally.h
bin_PROGRAMS		= sally

libsally_la_SOURCES	= sally.c sally.h util.c util.h murmur.c murmur.h

sally_SOURCES		= tool.c tool.h
sally_LDADD		= libsally.la
