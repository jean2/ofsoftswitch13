#! /usr/bin/env python

import sys
import os
import time

if(len(sys.argv)!=2):
        print '    -usage: ./start_switch_datapath.py  <dpctl-tcp-port>'
        print '   example: ./start_switch_datapath.py  6635'
        exit()
else:
	port = sys.argv[1]
 
os.system('clear')
print '***********STARTING SWITCH DATAPATH*************'
print
time.sleep(1)
start = 'sudo udatapath/ofdatapath --datapath-id=b4b52fd06906 --interfaces=eth0 ptcp:'+port
os.system(start)


