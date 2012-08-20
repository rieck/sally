#!/bin/sh 

BYTE_SETS="arts sprot"
WORD_SETS="enron rfc sentences"
RUNS=`seq 1 5`

# Sally experiments
OUT="results/python.time"
rm -rf $OUT

for SET in $WORD_SETS ; do 
    # Read data to utilize caching
    cat data/$SET.data > /dev/null
    
    for RUN in $RUNS ; do
        echo -n "$SET " >> $OUT
        /usr/bin/time -f 'time %U memory %M' -a -o $OUT \
            ./python.py data/$SET.data 3 "%0a%0d .,:;!?" > /dev/null
    done
done

for SET in $BYTE_SETS ; do 
    # Read data to utilize caching
    cat data/$SET.data > /dev/null
    
    for RUN in $RUNS ; do
        echo -n "$SET " >> $OUT
        /usr/bin/time -f 'time %U memory %M' -a -o $OUT \
            ./python.py data/$SET.data 3 "" > /dev/null
    done
done

