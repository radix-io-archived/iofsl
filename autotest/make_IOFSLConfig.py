#!/bin/env python

import ConfigParser, os, sys
config = ConfigParser.RawConfigParser()


# We can add sections for any identification 
# we wish to make. This file can be expanded as needed.
# add new sections to the top of the file.  the config file
# reverses the order in which the sections are displayed

# Section 1 
config.add_section('section1')
config.set('section1', 'git_repo', 'git')
config.set('section1', 'cvs_repo', 'cvs')

# Section 2
config.add_section('section2')
config.set('section2', 'git_command1', 'clone')
config.set('section2', 'git_command2', 'check')
config.set('section2', 'git_command3', 'diff')
config.set('section2', 'git_command4', 'commit')
config.set('section2', 'git_command5', 'push')
config.set('section2', 'git_command6', 'pull')
config.set('section2', 'git_command7', 'status')

# Section 3 git addresses
config.add_section('section3')
config.set('section3', 'git_address1', 'git://git.mcs.anl.gov')
config.set('section3', 'git_address2', 'git://git.kernel.org/pub/scm/git')
config.set('section3', 'git_address3', 'git://git.mcs.anl.gov/')
config.set('section3', 'git_address4', '')
config.set('section3', 'git_address5', '')

# Section 4 possible git directories
config.add_section('section4')
config.set('section4', 'git_directory1', '/iofsl.git')
config.set('section4', 'git_directory2', '/git.git')
config.set('section4', 'git_directory3', ':')
config.set('section4', 'git_directory4', ':')

# Section 5 build commands
config.add_section('section5')
config.set('section5', 'iofsl_prepare', '/iofsl/prepare')
config.set('section5', 'iofsl_configure', '/iofsl/configure')
config.set('section5', 'iofsl_wcunit', '--with-cunit=/users/rjdamore/opt/cunit-2.1 ')
config.set('section5', 'iofsl_wbmi', '--with-bmi=/users/rjdamore/opt/bmi-2.8.2 ')
config.set('section5', 'iofsl_wboost', '--with-boost=/users/rjdamore/opt/boost-1.44 ')
config.set('section5', 'iofsl_wmpi', '--with-mpi=/user/rjdamore/opt/mpichNoZoidfs ')
config.set('section5', 'iofsl_prefix', '--prefix=/users/rjdamore/iofsl-install')
config.set('section5', 'iofsl_make', 'iofsl/make')
config.set('section5', 'iofsl_make_install', 'iofsl/make install')

# Section 6 dependency directories
# indicate the location of the IOFSL dependencies...
config.add_section('section6')
config.set('section6', 'cunit_dir', '/opt/cunit-2.1')
config.set('section6', 'mpich_dir', '/opt/mpichNoZoidfs')
config.set('section6', 'bmi_dir', '/opt/bmi-2.8.2')
config.set('section6', 'boost_dir', '/opt/boost-1.44')


# Section 7 Miscellaneous variables
config.add_section('section7')
config.set('section7', '$HOME', '/users/rjdamore')
config.set('section7', 'ion', 'ion')
config.set('section7', 'server', '/iofsl-install/bin/iofwd')
config.set('section7', 'conf_cmd', '--config')
config.set('section7', 'serv_conf', '/iofsl/defaltconfig.cf')
config.set('section7', 'cunit_io_test', 'iofsl/test/unit-tests/zoidfs-io-cunit')
config.set('section7', 'cunit_md_test', 'iofsl/test/unit-tests/zoidfs-md-cunit')
config.set('section7', 'cunit_test_dir', 'tmp/iofsl-test-dir')

# Section 8 Environment variables
config.add_section('section8')
config.set('section8', 'ld_library_path', '/users/rjdamore/opt/mpichNoZoidfs/lib:\
/users/rjdamore/opt/boost-1.44/lib:/users/rjdamore/opt/cunit-2.1/lib:\
/users/rjdamore/opt/bmi-2.8.2/lib')
config.set('section8', 'cflags', '-I/users/rjdamore/iofsl/src/zoidfs')
config.set('section8', 'mpi_inc', '-lm')




with open('iofsl_values.cfg','w') as configfile:
	config.write(configfile)

