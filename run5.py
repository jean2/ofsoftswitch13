#! /usr/bin/env python

import os
import time

slp =0.5

os.system('clear')
print '********run5.py script**********'
print
print 'Using dpctl to invoke controller like messages' 

#Stats:print the current stats on the screen (should be empty)
print '********showing stats***********'
print 

os.system("utilities/dpctl tcp:ubuntu:6635 stats-flow table=0")

#Opening bundle 
print '********opening bundle**********'
print 
time.sleep(slp)
os.system("utilities/dpctl tcp:ubuntu:6635 bundle open -b 4")

#Adding an action to bundle 
print '********adding to bundle********'
print 
time.sleep(slp)
os.system("utilities/dpctl -b 4 tcp:ubuntu:6635 flow-mod table=0,cmd=add ,in_port=1, apply:")
time.sleep(slp)
os.system("utilities/dpctl -b 4 tcp:ubuntu:6635 flow-mod table=0,cmd=add ,in_port=2, apply:")
time.sleep(slp)
os.system("utilities/dpctl -b 4 tcp:ubuntu:6635 flow-mod table=0,cmd=add ,in_port=3, apply:")

#Stats:shold be empty
time.sleep(slp)
print '********showing stats***********'
print
os.system("utilities/dpctl tcp:ubuntu:6635 stats-flow table=0")

#Bundle Commit 
time.sleep(slp)
print '********bundle commit***********'
print
os.system("utilities/dpctl tcp:ubuntu:6635 bundle commit -b 4")
#os.system("utilities/dpctl tcp:ubuntu:6635 bundle commit -b 4 -f 4 -T 271828")

#Stats:shold be empty
time.sleep(slp)
print '********showing stats***********'
print

os.system("utilities/dpctl tcp:ubuntu:6635 stats-flow table=0")
