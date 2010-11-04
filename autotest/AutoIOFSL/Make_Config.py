'''
Created on Nov 2, 2010

@author: Rico
'''
from configobj import ConfigObj
config = ConfigObj()
config.filename = 'git_repo.cfg'

# We can add sections for identifying repos.  
#Section1
config['section1'] = {}
config['section1'] ['git']  = 'git '
config['section1'] ['cvs']  = 'cvs '

#Section2
config['section2'] = {}
config['section2'] ['git_command1']   = 'clone '
config['section2'] ['git_command2']   = 'check '
config['section2'] ['git_command3']   = 'diff '
config['section2'] ['git_command4']   = 'commit '
config['section2'] ['git_command5']   = 'push '
config['section2'] ['git_command6']   = 'pull '

#Section3
config['section3'] = {}
config['section3'] ['git_address1']   = 'git://git.mcs.anl.gov'
config['section3'] ['git_address2']   = 'git://git.kernel.org/pub/scm/git'
config['section3'] ['git_address3']   = 'git://git.mcs.anl.gov/'
config['section3'] ['git_address4']   = ''
config['section3'] ['git_address5']   = ''
#Section4
config['section4'] = {}
config['section4'] ['git_directory1'] = '/iofsl.git'
config['section4'] ['git_directory2'] = '/git.git'
config['section4'] ['git_directory3'] = ':'
config['section4'] ['git_directory4'] = ':'
config['section4'] ['git_directory5'] = ':'
#Write to our file
config.write()





