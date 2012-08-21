#!/bin/sh
SETS="enron sprot"

echo "Note: Sally needs to be compiled with --enable-evaltime"

# Sally experiments
for SET in $SETS ; do 
    OUT="results/sally-$SET.stats"
    rm -rf $OUT

    # Read data to utilize caching
    cat data/$SET.data > /dev/null
    
    sally -c data/$SET.cfg data/$SET.data /dev/null > $OUT
done
