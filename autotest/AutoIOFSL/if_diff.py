#!/usr/bin/env python
'''
Created on Nov 8, 2010

@author: Rico D'Amore
'''
import os,sys
from send_mail import Mail
import subprocess
from configobj import ConfigObj


config = ConfigObj('git_repo.cfg')

def diff_repo(): # check the repository, if difference exists, download new version.
   
    try:   
        retcode = subprocess.check_output(['git', 'status', "Documents/workspace/python/AutoIOFSL/iofsl" ],
                                          stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError:
            print"git command failed. maybe you are in the wrong directory?"
    else:
        o = open('downld_error_1.log', 'w')
        o.write(retcode)
        o.close()
        
        m = Mail()
        m.dwnld_mail()
        
        
  
        

            
def main():      
    diff_repo()
    
if __name__ == '__main__':main()

