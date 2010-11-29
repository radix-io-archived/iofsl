#!/usr/bin/env python
'''
Created on Nov 16, 2010

@author: Rico D'Amore
'''

from configobj import ConfigObj
import os, sys

config = ConfigObj('iofsl_config.cfg')
# WE use these classes to preform the IOFSL specific build commands for us.
class Prepare:
    
    try:
        prep   = config['section5']['iofsl_prepare']
        conf   = config['section5']['iofsl_configure']
        pref   = config['section5']['iofsl_prefix']
        wcunit = config['section5']['iofsl_wcunit']
        wbmi   = config['section5']['iofsl_wbmi']
        wboost = config['section5']['iofsl_wboost']
        wmpi   = config['section5']['iofsl_wmpi']
        
    except IOError: print "Configuration info was not retrieved properly."
    else:
        conf_cmd = (conf, wcunit, wbmi, wboost, wmpi, pref)
        configure = ("".join(conf_cmd))    
        
class Make:
    
    try:
        make      = config['section5']['iofsl_make']
        mkinstall = config['section5']['iofsl_make_install']
        
    except IOError: print "Make info was not retrieved properly."
    else:
        None
          
  
  

    

        

