#ifndef IOFWD_IOFWDMAIN_HH
#define IOFWD_IOFWDMAIN_HH

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

   // Called to initialize the server
   void boot ();

   void run (); 

   // Called when the server needs to shut down
   void shutdown (); 

protected:

}; 

//===========================================================================
}



#endif
