#!/usr/bin/env python
'''
Created on Nov 16, 2010

@author: Rico D'Amore
'''

from configobj import ConfigObj
import os, sys

config = ConfigObj('iofsl_config.cfg')
home = config['section7']['$HOME']

# WE use these classes to preform the IOFSL specific build commands for us.
class Prepare:
	def prepare(self):
		prepare   = config['section5']['iofsl_prepare']
		
		os.system(home + prepare)

class Configure:
	def configure(self):
		configure = config['section5']['iofsl_configure'] 
                wcunit    = config['section5']['iofsl_wcunit']
                wbmi      = config['section5']['iofsl_wbmi']
                wboost    = config['section5']['iofsl_wboost']
                wmpi      = config['section5']['iofsl_wmpi']
                wprefix   = config['section5']['iofsl_prefix']

		os.system(home + configure + " " + wcunit + " " + wbmi + " " + wboost + " " + wmpi + " " + wprefix) 

class Make:
        def make(self):
		#make      = config['section5']['iofsl_make']
		#mkinstall = config['section5']['iofsl_make_install']

		#os.system(home + make)
		#os.system(home + mkinstall)
          	os.system('make')
		os.system('make install')
  
  

    

        

