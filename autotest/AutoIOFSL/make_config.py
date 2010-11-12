#!/usr/bin/env python
'''
Created on Nov 2, 2010

@author: Rico D'Amore
'''
from configobj import ConfigObj
config = ConfigObj()
config.filename = 'iofsl_config.cfg'

# We can add sections for identifying repos.  
#Section1
config['section1'] = {}
config['section1'] ['git_repo']  = 'git '
config['section1'] ['cvs_repo']  = 'cvs '

#Section2
config['section2'] = {}
config['section2'] ['git_command1']   = 'clone '
config['section2'] ['git_command2']   = 'check '
config['section2'] ['git_command3']   = 'diff '
config['section2'] ['git_command4']   = 'commit '
config['section2'] ['git_command5']   = 'push '
config['section2'] ['git_command6']   = 'pull '
config['section2'] ['git_command7']   = 'status '

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
#Section5 ENV
#config['section5'] = {}
#config['section5'] ['CFLAGS']         = '-I$HOME/iofsl/src/zoidfs'
#config['section5'] ['MY_MPI_HOST']    = 'turing'
#config['section5'] ['MY_MPI_HOST']    = 'cielito'
#config['section5'] ['LD_LIBRARY_PATH']= '$HOME/opt/mpich2-1.3/lib/:$HOME/opt/boost-1.44/lib:/usr/lib64:/usr/lib64/libstdc++.so.64\
 #                                       :$HOME/opt/cunit-2.1/lib:$HOME/opt/bmi-2.8.2/lib'
#config['section5'] ['path']           = '$HOME/bin $HOME/git'




#Write to our file
config.write()





