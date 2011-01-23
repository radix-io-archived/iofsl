#!/usr/bin/env python

import time
import sys
import subprocess
import signal
import os
import errno

#
# Starts a server, listening on the loopback interface and runs the unit tests
# against it.
#

# Where we can find the server binary
SERVERBINARY = "src/iofwd/iofwd"

# Directory holding configuration files to test
CONFIGFILES = "local-configs"

# Test programs
TESTS = [ "test/unit-tests/zoidfs-io-cunit", "test/unit-tests/zoidfs-md-cunit" ]

errorcount = 0


def log (string):
   sys.stdout.write (string);
   sys.stdout.flush ()


def error (string):
   global errorcount
   errorcount += 1
   sys.stderr.write (string)

def signalAndWait (proc, signal, delay):
   ret = False
   log ("Sending signal " + str(signal) + " to " + str(proc.pid))
   starttime = time.time()
   while True:
      if proc.poll () != None:
         ret = True
         break

      if (time.time() - starttime) > delay:
         ret = False
         break
      try:
         os.kill (proc.pid, signal)
      except os.error,e:
         if e.errno == errno.ESRCH:
            # Proc stopped in the mean time
            ret = True
            break
      log (".")
      time.sleep (.66)
   log (".")
   if ret:
      log ("Stopped\n")
   else:
      log ("no effect\n")
   return ret
      
#============================================================================
#============================================================================
#============================================================================

class LocalServer (object):
   """Starts a local server, and tracks it"""

   def __init__ (self, configfile):
      self.configfile = configfile
      self.started = False
      pass

   def start(self):
      log ("Starting server...\n")
      try:
        self.serverproc = subprocess.Popen (
            [SERVERBINARY, "--config", self.configfile],
            close_fds=True)
        self.started = True
      except subprocess.OSError,a:
         error ("Could not start server: " + a)

   def waitReady (self):
      # @TODO: add code to make sure we only return when a server is actually
      # ready to serve clients. Either ping the server using zoidfs_null or
      # watch for certain output
      time.sleep (1)
      return True

   def isRunning (self):
      """Return true if the process is still running"""
      return self.serverproc.poll () != None

   def stop(self):
      assert self.started
      log ("Stopping server...\n")
      if self.serverproc.poll () != None:
         error ("Server already stopped??\n")
      else:
         log ("Trying to shutdown server...\n")
         if not signalAndWait (self.serverproc, signal.SIGINT, 6):
            error ("Server did not respond to shutdown. Killing server\n")
            signalAndWait (self.serverproc, signal.SIGKILL, 6)

   def didStart(self):
      return self.started

   def getExitCode (self):
      assert self.started
      return self.serverproc.returncode

   def getAddress (self):
      return "tcp://127.0.0.1:1234"

#============================================================================
#============================================================================
#============================================================================

class LocalTest:
   """Run a test application against an IOFSL server. Track it and kill if
      needed"""

   def __init__ (self, progandargs):
      self.program = progandargs
      self.started = False

   def start(self, location):
      log ("Starting test against " + location + "...\n")
      testEnv = os.environ
      testEnv["ZOIDFS_ION_NAME"] = location
      try:
        self.test = subprocess.Popen (self.program, close_fds=True,
              env=testEnv)
        self.started = True
      except OSError,a:
         error ("Could not start test: " + str(a) + "\n")

   def didStart(self):
      return self.started

   def forceStop(self):
      """Send SIGKILL to program"""
      if not signalAndWait (self.test, signal.SIGKILL, 6):
         error ("Could not stop test!\n")

   def isRunning (self):
      if not self.started:
         return self.started
      return self.test.poll () == None

   def getExitCode(self):
      assert self.started
      return self.test.returncode

#============================================================================
#============================================================================
#============================================================================

def runTest (testexec, configfile, maxtime):
   testresult = False
   myserver = LocalServer (configfile)
   mytest = LocalTest (testexec)
   try:
     myserver.start ()
     log ("Waiting for server to become ready\n")
     myserver.waitReady ()
     log ("Starting test\n")
     mytest.start (myserver.getAddress())
     starttime = time.time ()
     while True:
        if not mytest.isRunning():
           log ("Test done!\n")
           break
        if (time.time() - starttime) > maxtime:
           error ("Test did not finish within " + str(maxtime) + "seconds!\n")
           break
        #log ("Test running... " + str(time.time() - starttime) + 
        #         " seconds...\n")
        time.sleep (min (maxtime, 1))

     if mytest.isRunning():
        error ("Killing test!\n")
        mytest.forceStop ()

     if mytest.didStart():
        ret = mytest.getExitCode ()
        if ret:
           error ("Test returned non-zero exit code " + str(ret) + "\n")
     else:
        error ("Test never started!\n")

     testresult = True

   finally:
      if mytest.isRunning ():
         mytest.forceStop ()
      myserver.stop ()

   if not myserver.didStart ():
      error ("Server never started!\n")
   else:
      ecode = myserver.getExitCode ()
      if ecode != 0:
         error ("Server returned non-zero exit code (" + str(ecode) + ")\n")
         testresult = False
   if not mytest.didStart ():
      error ("Test never started!\n")
   else:
      if mytest.getExitCode () != 0:
         error ("Test returned non-zero exit code!\n")
         testresult = False
   return testresult


# get directory arguments if any. builddir followed by srcdir
top_builddir = "."
top_srcdir = "."
if (len(sys.argv) >= 3):
   (top_builddir, top_srcdir) = sys.argv[1:3]
   sys.stdout.write ("Builddir = " + top_builddir + "\n")
   sys.stdout.write ("Srcdir   = " + top_srcdir + "\n")

configfile = os.path.join(top_srcdir, "defaultconfig.cf")

# prepare test directory
import tempfile
import shutil

testdir = tempfile.mkdtemp ()
log ("Using temporary directory " + testdir + "\n")
try:
  runTest ([os.path.join(top_builddir, "test/unit-tests/zoidfs-io-cunit"),
     testdir], configfile, 60)
finally:
  shutil.rmtree (testdir)

os.mkdir (testdir)
try:
   runTest ([os.path.join(top_builddir, "test/unit-tests/zoidfs-md-cunit"),
      testdir], configfile, 60)
finally:
   shutil.rmtree (testdir)

sys.exit (errorcount)


