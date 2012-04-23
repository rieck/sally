# Sally - A Tool for Embedding Strings in Vector Spaces
# Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)

EXTRA_DIST              = sally.h.in
BUILT_SOURCES           = sally.h

SUBDIRS 		= input output fvec .
INCLUDES                = -I$(top_srcdir)/src/input \
			  -I$(top_srcdir)/src/output \
			  -I$(top_srcdir)/src/fvec

bin_PROGRAMS		= sally
sally_SOURCES		= sally.c
sally_DEPENDENCIES    	= sally.h libsally.la
sally_LDADD		= libsally.la

noinst_LTLIBRARIES     	= libsally.la
libsally_la_SOURCES	= util.c util.h sconfig.c \
			  sconfig.h common.h uthash.h murmur.c \
			  murmur.h md5.c md5.h 
libsally_la_LIBADD	= input/libinput.la \
			  output/liboutput.la \
			  fvec/libfvec.la

sally.h: sally.h.in
	sed -e 's|__SYSCONFDIR__|$(sysconfdir)|' \
		$(top_srcdir)/src/sally.h.in > sally.h

clean-local:
	rm -f sally.h

beautify:
	gindent -i4 -npsl -di0 -br -d0 -cli0 -npcs -ce -nfc1 -nut \
		-T fvec_t \
		$(libsally_SOURCES) $(sally_SOURCES)
	