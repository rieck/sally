# Sally - A Library for String Features and String Kernels
# Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)

man1_MANS = sally.man 
EXTRA_DIST = sally.pod

sally.man: sally.pod
	pod2man -c "User Manual" -s 1 -r "sally $(VERSION)" $< > $@

distclean-local:
	rm -f sally.man
		
dist-hook: distclean-local sally.man
