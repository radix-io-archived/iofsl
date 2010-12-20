#!/bin/env python
'''
Created on Dec 8, 2010
@author Rico D'Amore
'''

import os,sys

from configobj import ConfigObj

config = ConfigObj('iofsl_config.cfg')

def run_cunit_md():
	home       = config['section7']['$HOME']
	ion        = config['section7']['ion']
	test_dir   = config['section7']['cunit_test_dir']
	cunit_test = config['section7']['cunit_md_test']

	f=open(home + ion, 'r')
	for line in f:
		node = line

	os.environ['ZOIDFS_ION_NAME']=node
	os.system(home + cunit_test + " " + home + test_dir)

def main():
	run_cunit_md()

if __name__ == '__main__':main()
