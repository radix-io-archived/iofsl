#!/usr/bin/env python
'''
Created on Nov 10, 2010

@author: Rico D'Amore
'''
import os,sys
import subprocess
from send_mail import Mail

def prep_config():
    # Prepare and configure the iofsl package, if there is a problem with configure, email the 
    # development list.  Use the config file for our desired dependencies.
    try: 
        os.system("../../prepare")
        os.system("../../configure > configure_error.log")
    except OSError:
        print "the configure script didn't execute, are you pointing to the correct directory?"
    else: 
        None
    
    try:
        fe = open('configure_error.log').read()
    except IOError: 
        print "Failed to open configure_error.log"    
        if "complete" in fe:
            None
        else:
            m = Mail()
            m.configure_mail()
      
        
#def make_iofsl():
    # Make the iofsl package and make install it in the desired directory.
    # Use the config file for our desired env
        
        




def main():
    prep_config()
    
if __name__ == '__main__':main()       
