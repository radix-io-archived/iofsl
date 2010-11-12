'''
Created on Nov 2, 2010

@author: Rico
'''
from configobj import ConfigObj
import os

config = ConfigObj('iofsl_config.cfg')
repoCount = 0
class Repo1:
        repoCount = 0
        try:  #retrieve our repo data from git_repo.cfg
            vc_command    = config['section1']['git_repo']
            git_command   = config['section2']['git_command1']
            git_address   = config['section3']['git_address1']
            git_directory = config['section4']['git_directory1']
        
        except IOError: print "The repo info was not retrieved correctly"
        
        else: #make a list item from the repo data
            repo1 = (vc_command, git_command, git_address, git_directory)
            repoCount = repoCount + 1 # increment our repo count
            
        def dwnldRepo1(self):
            os.system("".join(Repo1.repo1))
        #    print "".join(Repo1.repo1)
        #    print "The Repo Count is" + " " + (Repo1.repoCount).__str__()
            
            
           
        
class Repo2:
        repoCount = Repo1.repoCount  # get our repo count from class Repo1:
        try:
            vc_command    = config['section1']['git_repo']
            git_command   = config['section2']['git_command1']
            git_address   = config['section3']['git_address2']
            git_directory = config['section4']['git_directory2']     
        
        except IOError: print "the repo info was not retrieved correctly"

        else:
            repo2 = (vc_command, git_command, git_address, git_directory)
            repoCount = repoCount + 1
        
        def dwnldRepo2(self):
            os.system("".join(Repo2.repo2))
         #   print "".join(Repo2.repo2)
         #  print "The Repo Count is" + " " + (Repo2.repoCount).__str__()
            
        
        
    

    

