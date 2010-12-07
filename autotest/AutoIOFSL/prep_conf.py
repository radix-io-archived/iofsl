#!/usr/bin/env python
'''
Created on Nov 10, 2010

@author: Rico D'Amore
'''
import os,sys,subprocess,logging


from os import path
from iofsl_const_objects import Prepare, Configure, Make
from send_mail import Mail
from configobj import ConfigObj

config = ConfigObj('iofsl_config.cfg')
home = config['section7']['$HOME']

# Check for all the dependencies that we need in: $HOME/opt.  If everything is good,
# then proceed to prepare and configure the IOFSL package.

def check_deps():  
    if os.path.isdir(home + '/opt/cunit-2.1') == True:
        pass
    else:
        print "Couldn't detect a valid CUNIT installation.  Please download CUNIT and install in $HOME/opt. Thank you."
        sys.exit()
    if os.path.isdir(home + '/opt/mpich2-1.3') == True:
        pass
    else:
        print "Couldn't detect a valid MPI installation.  Please download MPICH2 and install in $HOME/opt.  Thank you."
        sys.exit()
    if os.path.isdir(home + '/opt/bmi-2.8.2') == True:
        pass
    else:
        print "Couldn't detect a valid BMI installation.  Please download BMI and install in $HOME/opt. Thank you."
        sys.exit()
    if os.path.isdir(home + '/opt/boost-1.44') == True:
        print " your dependencies are ok"
        prep_config()
    else:
        print "Couldn't detect a valid BOOST installation. Please download BOOST and install in $HOME/opt. Thank you."
        sys.exit()   

def prep_config():
    # Prepare and configure the iofsl package, if there is a problem with configure, email the 
    # development list.  Use the config file to point to our dependencies.
    try: 
       p = Prepare()
       p.prepare() 
       # os.system("./prepare")
        
    except OSError:
        print "the prepare script didn't execute, are you pointing to the correct directory?"
    
    else: 
        
	c = Configure()
	c.configure()

	#os.system("./configure") 
        
     # If configure is successful we will have a makefile located in the cwd.
     # Continue on to the make and make_install functions
    if os.path.isfile(home + '/iofsl/autotest/AutoIOFSL/Makefile') == True:
            print "your configuration was a success!"
            
	    m = Make()
	    m.make()
    else:
            m = Mail()
            m.configure_mail()
            
def make():
    try:
       m = Make()
       m.make()
    except OSError:
	print "The make command failed, please review the config file"
    else:
	print "make and make install were a success"	
    #Make and install the IOFSL distro.
#    try:
 #       os.system(Make.make)
  #  except OSError:
   #     print "the make command failed, please review the config file"
   # else:
    #    print "make was a success!"
        
     #   try:
      #      os.system(Make.mkinstall)
      #  except OSError:
       #     print "the make_install command failed please check the config file"
       # else:
        #    print "make_install was a success!"
            
        
def main():
    check_deps()
    
if __name__ == '__main__':main()       
