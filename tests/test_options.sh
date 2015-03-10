#!/bin/sh
# Sally - A Tool for Embedding Strings in Vector Spaces
# Copyright (C) 2010-2014 Konrad Rieck (konrad@mlsec.org);
# --
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3 of the License, or (at your
# option) any later version.  This program is distributed without any
# warranty. See the GNU General Public License for more details.
# --
# Simple test comparing precomputed output for different options
# against compiled version of Sally
#

# Check for directories
test -z "$TMPDIR" && TMPDIR="/tmp"
test -z "$BUILDDIR" && BUILDDIR=".."
test -z "$SRCDIR" && SRCDIR=".."

DATA=$SRCDIR/tests/strings.txt
TEST=$SRCDIR/tests/test_options.txt
SALLY=$BUILDDIR/src/sally
OUTPUT=$TMPDIR/sally-$$.txt
rm -f $OUTPUT

# Loop over some random options
for OPTION in "--decode_str" "--reverse_str" "-n 3" "-n 1" "-p" \
              "-g bytes" "-B" "-s -n 3" "-E bin" "-E cnt" \
              "-N l1" "-N l2" "-S" "-b 12" "-b 24" "-r simhash" \
              "-r minhash" ; do
    echo "$OPTION" >> $OUTPUT
    $SALLY $OPTION $DATA - | grep -v -E '^#' >> $OUTPUT
done

# Save output
#cp $OUTPUT /tmp/test_options.txt

# Diff output and precomputed data
diff $TEST $OUTPUT
RET=$?

# Clean up and exit
rm -f $OUTPUT
exit $RET
