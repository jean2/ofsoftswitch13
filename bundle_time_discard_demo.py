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
#Opening bundle
print '***********Open Bundle******************'
time.sleep(slp)
open_bundle = 'utilities/dpctl tcp:'+host+':'+port+' bundle open -b 4'
print 'dpctl usage:'
print open_bundle
os.system(open_bundle)
print
#Adding an action to bundle 
print '***********Bundle Add msg***************'
time.sleep(slp)
print 'dpctl usage:'
add_bundle = 'utilities/dpctl -b 4 tcp:'+host+':'+port+' flow-mod table=0,cmd=add ,in_port=1, apply:'
print add_bundle
print add_bundle
print add_bundle
os.system(add_bundle)
time.sleep(slp)
os.system(add_bundle)
time.sleep(slp)
os.system(add_bundle)
time.sleep(slp)
#Bundle Commit 
time.sleep(slp)
delta_time = 5
print '***********Bundle Commit in '+str(delta_time)+' sec*******'
commit_time   = "%.9f" % time.time()
commit_time   = commit_time.split('.')
commit_time[0] = str(int(commit_time[0])+delta_time)
commit_in_time = 'utilities/dpctl tcp:'+host+':'+port+' bundle commit -b 4 -f 4 -T '+commit_time[0]+' -N '+commit_time[1]
print 'dpctl usage:'
print commit_in_time
os.system(commit_in_time)
print
#Stats:shold be empty
time.sleep(slp)
print '***********Showing empty stats**********'
print 'dpctl usage:'
print show_stat
os.system(show_stat)
print 
#Discard:after 2 seconds (before commit time)
print 'Couting '+str(delta_time)+' sec until commit is done'
discard = 'utilities/dpctl tcp:'+host+':'+port+' bundle discard -b 4'
for i in range(1,delta_time+1):
	time.sleep(1)
	if i==2:
		os.system(discard)
	print i
print '***********Showing empty stats*******'
print 'dpctl usage:'
print show_stat
os.system(show_stat)


