#!/bin/sh 

SETS="arts enron rfc sentences sprot"
RUNS=`seq 1 5`

# Sally experiments
OUT="results/sally.time"
rm -rf $OUT

for SET in $SETS ; do 
    # Read data to utilize caching
    cat data/$SET.data > /dev/null
    
    for RUN in $RUNS ; do
        echo -n "$SET " >> $OUT
        /usr/bin/time -f 'time %U memory %M' -a -o $OUT \
            sally -c data/$SET.cfg data/$SET.data /dev/null 
    done
done
