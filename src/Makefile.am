# Sally - A Tool for Embedding Strings in Vector Spaces
# Copyright (C) 2010-2012 Konrad Rieck (konrad@mlsec.org)

EXTRA_DIST              = sally.h.in
BUILT_SOURCES           = sally.h

SUBDIRS 		= input output libsally .
INCLUDES                = -I$(top_srcdir)/src/input \
			  -I$(top_srcdir)/src/output \
			  -I$(top_srcdir)/src/libsally

bin_PROGRAMS		= sally
sally_SOURCES		= sally.c util.c util.h sconfig.c \
			  sconfig.h common.h uthash.h murmur.c \
			  murmur.h md5.c md5.h 
sally_DEPENDENCIES    	= sally.h 
sally_LDADD		= input/libinput.la \
			  output/liboutput.la \
			  libsally/libsally.la

sally.h: sally.h.in
	sed -e 's|__SYSCONFDIR__|$(sysconfdir)|' \
		$(top_srcdir)/src/sally.h.in > sally.h

clean-local:
	rm -f sally.h
	