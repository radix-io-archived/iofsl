#!/bin/env python
'''
Created on Dec 8, 2010
@author Rico D'Amore
'''

import os,sys,signal,ConfigParser


config = ConfigParser.ConfigParser()
path = os.getcwd()
config.readfp(open( path + '/' + 'iofsl_values.cfg'))

def run_cunit_io():
	home       = config.get('section7', '$HOME')
	ion        = config.get('section7', 'ion')
	test_dir   = config.get('section7', 'cunit_test_dir')
	cunit_test = config.get('section7', 'cunit_io_test')

	f=open(home + ion, 'r')
	for line in f:
		node = line	

	os.environ['ZOIDFS_ION_NAME']=node
	# Here we try ad run the MD test, we want to create an error report
	# if the test hangs.
	try:
		def handler(signum, frame):
			print 'We call the handler with the signal',signum
			raise IOError("The test is hanging we are aborting")
			m = Mail()
			m.configure_mail()

		signal.signal(signal.SIGALRM, handler)
		signal.alarm(20)	

	        os.system(home + cunit_test + " " + home + test_dir)
	
	except OSError:
		if signal.CTRL_C_EVENT:
			print "The test didn't execute"
		else:
			pass
	else:
		if signal.CTRL_C_EVENT:
			print "The test didn't execute correctly!"
			
def main():
	run_cunit_io()
	
if __name__ =='__main__':main()
