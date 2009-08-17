#include "signals.hh"

namespace iofwdutil
{
//===========================================================================

void disableAllSignals (bool debug)
{
   sigset_t set; 
   sigfillset (&set); 
   if (debug)
   {
      /* allow ctrl-c */
      sigdelset (&set, SIGINT);
   }
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
