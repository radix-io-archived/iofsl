#ifndef IOFWD_IOFWDMAIN_HH
#define IOFWD_IOFWDMAIN_HH

#include <memory>
#include "frontend/IOFWDFrontend.hh"
#include "iofwdutil/zlog/ZLogSource.hh"


namespace iofwd
{
//===========================================================================


/**
 * Main object; Represents the whole I/O forwarding server
 * Groups resources and provides a single startup/shutdown point
 */
class IOFWDMain 
{
public:

   IOFWDMain (); 

   // Called to initialize the server
   void boot ();

   void run (); 

   // Called (possibly from other thread) when the server needs to shut down
   void shutdown (); 

protected:
   std::auto_ptr<frontend::Frontend> frontend_;  

   iofwdutil::zlog::ZLogSource & mainlog_; 
}; 

//===========================================================================
}



#endif
