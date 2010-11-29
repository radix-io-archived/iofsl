#!/usr/bin/env python
'''
Created on Nov 2, 2010

@author: Rico D'Amore
'''
import os,sys,re,datetime,math,random
from repo_objects import Repo1,Repo2
from optparse import OptionParser


def options_parse():
    parser = OptionParser()
    parser.add_option("-r", "--repo", action="store", type="string", dest="repository_num")

       
   # (options, args) = parser.parse_args(["-rgit2"])
    (options, args) = parser.parse_args(sys.argv)
    print options.repository_num
    
    g = options.repository_num
    if g == "git1":
        is_ok1()
    elif g == "git2":
        is_ok2()
    else:   
        print "Error: your input repo value doesn't exist in the Configuration file""\n\
              format = -r ""repo-name"" ie: -r git1, -r git2, -r git3, -r cvs1 etc.."  


def is_ok1():
        x1 = Repo1()
        x1.dwnldRepo1()
def is_ok2():
        x1 = Repo2()
        x1.dwnldRepo2()
            
        
def main():
    options_parse()
    
    
if __name__ == '__main__':main()

