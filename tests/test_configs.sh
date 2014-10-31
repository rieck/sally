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
# Simple test comparing precomputed output for different configurations
# against compiled version of Harry
#

# Check for directories
test -z "$TMPDIR" && TMPDIR="/tmp"
test -z "$BUILDDIR" && BUILDDIR=".."
test -z "$SRCDIR" && SRCDIR=".."

DATA=$SRCDIR/tests/strings.txt
TEST=$SRCDIR/tests/test_configs.txt
SALLY=$BUILDDIR/src/sally
OUTPUT=$TMPDIR/sally-$$.txt
rm -f $OUTPUT

# Loop over config files
for CONFIG in config1.cfg config2.cfg config3.cfg ; do
    echo $CONFIG >> $OUTPUT
    $SALLY -c $SRCDIR/tests/$CONFIG $DATA - | grep -v -E '^#' >> $OUTPUT
done

# Save output
cp $OUTPUT /tmp/test_configs.txt

# Diff output and precomputed data
diff $TEST $OUTPUT
RET=$?

# Clean up and exit
rm -f $OUTPUT
exit $RET
