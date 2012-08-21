#!/bin/sh 

BYTE_SETS="sprot arts"
WORD_SETS="enron rfc"
RUNS=`seq 1 5`

# Octave experiments
OUT="results/matlab.time"
rm -rf $OUT

for SET in $WORD_SETS ; do 
    # Read data to utilize caching
    cat data/$SET.data > /dev/null
    
    for RUN in $RUNS ; do
        echo -n "$SET " >> $OUT
        FILE="'data/$SET.data'"        
        /usr/bin/time -f 'time %U memory %M' -a -o $OUT \
            matlab -nodisplay -nojvm -nodesktop \
                   -r "matlab($FILE,'/dev/null', 5, '%0a%0d .,:;?!'); exit" 
    done
done

for SET in $BYTE_SETS ; do 
    # Read data to utilize caching
    cat data/$SET.data > /dev/null
    
    for RUN in $RUNS ; do
        echo -n "$SET " >> $OUT
        FILE="'data/$SET.data'"
        /usr/bin/time -f 'time %U memory %M' -a -o $OUT \
            matlab -nodisplay -nojvm -nodesktop \
                   -r "matlab($FILE,'/dev/null', 5, ''); exit" 
    done
done
