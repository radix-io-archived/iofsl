#include "signals.hh"
#include "assert.hh"

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
   err =  sigwait (set, &sig);
   ALWAYS_ASSERT(err == 0);
   return sig;
}
//===========================================================================
}
