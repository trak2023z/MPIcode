# -*- encoding: utf-8 -*-
import sys
import random
if __name__ == "__main__": 
    if(len(sys.argv)<5):
        sys.exit("2 file names need and 3 sizes for matrix")
    
    f1=open(str(sys.argv[2]),"w")
    f2=open(str(sys.argv[1]),"w")
    
    x1=int(sys.argv[3])
    y1=int(sys.argv[4])
    
    x2=int(sys.argv[4])
    y2=int(sys.argv[5])
    
    for i in xrange(0,x1):
        for j in xrange(0,y1):
            f1.write(str(round(random.uniform(-1.0, 1.0),2)))
            if j!=y1-1: f1.write(" ")
        if i!=x1-1: f1.write('\n')
        
    for i in xrange(0,x2):
        for j in xrange(0,y2):
            f2.write(str(round(random.uniform(-1.0, 1.0),2)))
            if j!=y2-1: f2.write(" ")
        if i!=x2-1: f2.write('\n')
    
    f1.close()
    f2.close()
    