# -*- encoding: utf-8 -*-
import sys
from random import randint
if __name__ == "__main__": 
    permutations=10
    print"{"
    for i in xrange(0, permutations):
        print"{",
        for j in xrange(0, permutations):
            if i==j:
                print "0",
            else:
                print randint(1,9),
            if j<permutations-1:
                print ",",
        print "},"
    print"}"