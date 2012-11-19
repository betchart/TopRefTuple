#!/usr/bin/python

import sys
if len(sys.argv)<4 :
    print "usage:",sys.argv[0]," jsonFile firstRun lastRun"
    sys.exit()

jsonFile = sys.argv[1]
firstRun = eval(sys.argv[2])
lastRun =  eval(sys.argv[3])

json = eval(open(jsonFile).readline())
outJson = {}
for run in json:
    if firstRun <= eval(run) <= lastRun:
        outJson[run] = json[run]
ordered = sorted([pair for pair in outJson.iteritems()], key = lambda i: eval(i[0]))

print "{%s}"%(', '.join(['"%s": %s'%(i,str(j)) for i,j in ordered]))

