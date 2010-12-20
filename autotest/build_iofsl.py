#!/usr/bin/env python
'''
Created on Dec 17, 2010
@author: Rico D'Amore
rjdamore@lanl.gov
'''

import os,sys,subprocess,ConfigParser
from os import path

config = ConfigParser.ConfigParser()
config.readfp(open('/users/rjdamore/iofsl/autotest/iofsl_values.cfg'))
home = config.get('section7', '$home')

# Execute a git pull, if anyhing has changed we will be updated.
def diff_repo(): 
    try:
	pull = config.get('section2', 'git_pull')
        os.system(pull)
    except IOError:
	print " git command failed, maybe you are in the wrong directory?"
    else:
	check_deps()	

# Check for all the dependencies that we need in $HOME/opt.  If everything is good,
# then proceed to prepare and configure the IOFSL pakage.

def check_deps():
    cunit = config.get('section6', 'cunit_dir')
    mpi   = config.get('section6', 'mpich_dir')
    bmi   = config.get('section6', 'bmi_dir')
    boost = config.get('section6', 'boost_dir')
	
    if os.path.isdir(home + cunit) == True:
        pass
    else:
        print "Couldn't detect a valid CUNIT installation.  Please download CUNIT and install in desired directory, and indicate that directory in make_IOFSL.cfg. Thank you." 
	sys.exit()
    if os.path.isdir(home + mpi) == True:
        pass
    else:
        print "Couldn't detect a valid MPI installation.  Please download MPI and install in desired directory, and indicate that directory in make_IOFSL.cfg. Thank you."
        sys.exit()
    if os.path.isdir(home + bmi) == True:
        pass
    else:
        print "Couldn't detect a valid BMI installation.  Please download BMI and install in desired directory, and indicate that directory in make_IOFSL.cfg. Thank you."
        sys.exit()
    if os.path.isdir(home + boost) == True:
	print "Your dependencies checkout okay: moving on to configure"
	prep_config()
    else:
	print "Couldn't detect a valid BOOST installation.  Please download BOOST and install in desired directory, and indicate that directory in make_IOFSL.cfg. Thank you."
        sys.exit()

def prep_config():
    # Prepare and configure the iofsl package, if there is a problem with configure, email the 
    # development list.  Use the config file to point to our dependencies.   
    # The mail script can be edited to mail to the user instead
    
    try:
        p = Prepare()
	p.prepare()
    except OSError:
	print "The prepare script didn't execute."
    else:
        c = Configure()
	c.configure()

    # If configure is successful we will have a mekfile located in the cwd.
    # Continue on to the make and make_install functions.
    
    if os.path.isfile(home + '/iofsl/Makefile') == True:
        print " Your configuration was a success!"
        
        m = Make()
        m.make()
    else:
        m = Mail()
        m.configure_mail()

class Prepare:
        def prepare(self):
                prepare = config.get('section5', 'iofsl_prepare')
                os.system(home + prepare)

class Configure:
        def configure(self):
                configure = config.get('section5', 'iofsl_configure')
                wcunit    = config.get('section5', 'iofsl_wcunit')
                wbmi      = config.get('section5', 'iofsl_wbmi')
                wboost    = config.get('section5', 'iofsl_wboost')
                wmpi      = config.get('section5', 'iofsl_wmpi')
                wprefix   = config.get('section5', 'iofsl_prefix')

                os.system(home + configure + " " + wcunit + " " + wbmi + " " + wboost + " " + wmpi + " " + wprefix)

class Make:
        def make (self):
                os.system('make')
                os.system('make install')

class Mail:
        def dwnld_mail(self):
	    os.system("")

	def configure_mail(self):
	    os.system("./autotest/configure_error.sh")

def main():
    diff_repo()

if __name__ == '__main__':main()
