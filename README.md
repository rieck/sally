![Sally](sally.png) 

Sally - A Tool for Embedding Strings in Vector Spaces
==

Introduction
-- 

Sally is a small tool for mapping a set of strings to a set of vectors. 
This mapping is referred to as embedding and allows for applying
techniques of machine learning and data mining for analysis of string
data.  Sally can be applied to several types of strings, such as text
documents, DNA sequences or log files, where it can handle common formats
such as directories, archives and text files of string data.

Sally implements a standard technique for mapping strings to a vector
space that is often referred to as vector space model or bag-of-words
model.  The strings are characterized by a set of features, where each
feature is associated with one dimension of the vector space.  The
following types of features are supported by Sally: bytes, words, n-grams
of bytes and n-grams of words.

Sally proceeds by counting the occurrences of the specified features in
each string and generating a sparse vector of count values. 
Alternatively, binary or TF-IDF values can be computed and stored in the
vectors.  Sally then normalizes the vector, for example using the L1 or L2
norm, and outputs it in a specified format, such as plain text or in
LibSVM or Matlab format.

Consult the manual page of Sally for more information.

Dependencies
--

+   zlib >= 1.2.1, <http://www.zlib.net/>
+   libconfig >= 1.3.2, <http://www.hyperrealm.com/libconfig/>
+   libarchive >= 2.70,  <http://libarchive.github.com/>

#### Debian & Ubuntu Linux

The following packages need to be installed for compiling Sally on Debian
and Ubuntu Linux

    gcc 
    libz-dev
    libconfig8-dev
    libarchive-dev 

For bootstrapping Sally from the GIT repository or manipulating the
automake/autoconf configuration, the following additional packages are
necessary.

    automake 
    autoconf 
    libtool

#### Mac OS X 

For compiling Sally on Mac OS X a working installation of Xcode is required
including `gcc`.  Additionally, the following packages need to be installed
via Homebrew

    libconfig   
    libarchive (from homebrew-alt) 

#### OpenBSD

For compiling Sally on OpenBSD the following packages are required. Note
that you need to use `gmake` instead of `make` for building Sally.

    gmake
    libconfig
    libarchive

For bootstrapping Sally from the GIT repository, the following packages
need be additionally installed

    autoconf
    automake
    libtool

Compilation & Installation
--

From GIT repository first run

    $ ./bootstrap

From tarball run

    $ ./configure [options]
    $ make
    $ make check
    $ make install

Options for configure

    --prefix=PATH           Set directory prefix for installation

This feature enables support for OpenMP in Sally. It is still
experimental.  Sally will execute certain parts of the processing in
parallel making use of multi-core architectures where possible.
 
    --enable-md5hash        Enable MD5 as alternative hash

Sally uses a hash function for mapping different features to
different dimensions in the vector space.  By default the very
efficient Murmur hash is used for this task.  In certain critical
cases it may be useful to use a cryptographic hash as MD5.

Copyright (C) 2010-2013 Konrad Rieck (konrad@mlsec.org);
			Christian Wressnegger (christian@mlsec.org);
			Alexander Bikadorov (abiku@cs.tu-berlin.de)
