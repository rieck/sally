# Sally - A Tool for Embedding Strings in Vector Spaces
# Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)

man1_MANS = sally.man 
EXTRA_DIST = sally.pod doxygen.cfg 

sally.man: 	sally.pod
		pod2man -c "User Manual" -s 1 -r "sally $(VERSION)" $< > $@

changes:	
		./git22changes.py

manpage:	sally.pod
		pod2text -l $< > ../MANPAGE
		rm -f ../MANPAGE.bak

distclean-local:
	rm -f sally.man
		
dist-hook: 	distclean-local sally.man manpage changes
