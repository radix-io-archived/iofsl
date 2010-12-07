#!/bin/env python	
'''
Created on Dec 1, 2010

@author Rico D'Amore
'''

import os,sys,re,platform
from configobj import ConfigObj

config = ConfigObj('iofsl_config.cfg')

def start_server():
	node = platform.node()
	ION_NAME = re.match('(.*?)(.)(.*?)(\\.)',node)
	ZOIDFS_ION_NAME = "tcp://"+node.split('.')[0]+":12600"
	os.environ['ZOIDFS_ION_NAME']=ZOIDFS_ION_NAME
	home = config['section7']['$HOME']
        
	f=open(home + "ion",'w')
	f.write(ZOIDFS_ION_NAME)
	f.close()
	
	server    = config['section7']['server']
	conf_cmd  = config['section7']['conf_cmd']
	serv_conf = config['section7']['serv_conf']
	os.system(home + server + " " + conf_cmd + " " + home + serv_conf)
	
def main():
	start_server()

if __name__ =='__main__':main()
