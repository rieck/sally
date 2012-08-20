# Sally - A Tool for Embedding Strings in Vector Spaces
# Copyright (C) 2010 Konrad Rieck (konrad@mlsec.org)

man1_MANS = sally.man 
doc_DATA = example.cfg

EXTRA_DIST = sally.pod doxygen.cfg sally.pdf sally.txt \
             latex-pre.inc.in latex-post.inc example.cfg

changes:	
	./git2changes.py

sally.man: sally.pod
	pod2man -c "User Manual" -s 1 -r "sally $(VERSION)" $< > $@

sally.txt: sally.pod
	pod2text -w 70 sally.pod > $@

sally.pdf: sally.pod latex-pre.inc latex-post.inc
	pod2latex -prefile $(top_builddir)/doc/latex-pre.inc \
	          -postfile $(top_srcdir)/doc/latex-post.inc \
	          -full -out $(top_builddir)/doc/sally.tex \
	          $(top_srcdir)/doc/sally.pod
	pdflatex $(top_builddir)/doc/sally.tex
	pdflatex $(top_builddir)/doc/sally.tex	
	pdflatex $(top_builddir)/doc/sally.tex	
	rm -f *.tex *.log *.aux *.toc

latex-pre.inc: latex-pre.inc.in
	sed -e 's|__VERSION__|$(VERSION)|' \
		$(top_srcdir)/doc/latex-pre.inc.in \
		> $(top_builddir)/doc/latex-pre.inc

distclean-local:
	rm -f sally.man sally.txt sally.pdf latex-pre.inc
		
dist-hook: distclean-local sally.man sally.txt sally.pdf changes
