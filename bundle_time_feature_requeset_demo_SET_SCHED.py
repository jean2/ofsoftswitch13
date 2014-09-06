#! /usr/bin/env python

import sys
import os
import time

if(len(sys.argv)!=3):
        print '    -usage: ./bundle_time_demo.py <computer_name/ip> <dpctl-port>'
        print '  exapmle : ./bundle_time_demo.py 192.168.1.18 6635'
	print '  exapmle : ./bundle_time_demo.py ubuntu 6635'

        exit()
else:
	host = sys.argv[1]
	port = sys.argv[2]
 
slp =1

os.system('clear')
print '***********Bundle Time Demo*************'
print
time.sleep(1)
#Show stats
print '***********Showing table 0 stats********'
show_stat='utilities/dpctl tcp:'+host+':'+port+' stats-flow table=0'
print 'dpctl usage:'
print show_stat
os.system(show_stat)
print 
#Multi part message meter-features exp
print '***********meter-features exp******************'
time.sleep(slp)
meter_features = 'utilities/dpctl tcp:'+host+':'+port+' -f2 bundle-feature'

print 'dpctl usage:'
print meter_features
os.system(meter_features)
print
