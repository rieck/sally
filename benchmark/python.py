#!/usr/bin/python

import os, sys, string, urllib

def extract_ngrams(str, nlen):
    """ Extract byte n-grams from a string """
    ngrams = {}

    # Extract n-grams directly
    for i in range(len(str) - nlen + 1):
        ngram = str[i:i + nlen]
        hash = ngram.__hash__() % 2**24

        if hash not in ngrams:
            ngrams[hash] = 0.0
        ngrams[hash] += 1.0

    return ngrams

def extract_wgrams(str, nlen, delim):
    """ Extract word n-grams from a string """
    ngrams = {}

    # Unify delimiters for splitting
    trans = string.maketrans(delim, delim[0] * len(delim))
    str = str.translate(trans)

    # Split string and remove empty words
    list = str.split(delim[0])
    list = filter(lambda x: len(x) > 0, list) 

    # Extract n-grams
    for i in range(len(list) - nlen + 1):
        ngram = delim[0].join(list[i:i + nlen])
        hash = ngram.__hash__() % 2**24

        if hash not in ngrams:
            ngrams[hash] = 0.0
        ngrams[hash] += 1.0
        
    return ngrams

def print_libsvm(ngrams, label):
    """ Print feature vector in libsvm format """
    print '%d' % label,
    
    # Print features sorted by dimension
    for feat in sorted(ngrams.items()):
        print '%d:%g' % feat,
    print

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print "Usage: %s <input> <nlen> <delim>" % sys.argv[0]
        sys.exit(0)

    nlen = int(sys.argv[2])
    delim = urllib.unquote(sys.argv[3])
    for line in open(sys.argv[1]):
        if len(delim) == 0:
            n = extract_ngrams(line, nlen)
        else:
            n = extract_wgrams(line, nlen, delim)
        print_libsvm(n, 1)
            
