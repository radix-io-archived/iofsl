#include "signals.hh"

namespace iofwdutil
{
//===========================================================================

void disableAllSignals ()
{
   sigset_t set; 
   sigfillset (&set); 
   pthread_sigmask (SIG_BLOCK, &set, 0); 
   //sigprocmask (SIG_BLOCK, &set, 0); 
}

int waitSignal (const sigset_t * set)
{
   int err; 
   int sig; 
   if ((err=sigwait (set, &sig)) > 0)
      throw err; 
   return sig; 
}
//===========================================================================
}
