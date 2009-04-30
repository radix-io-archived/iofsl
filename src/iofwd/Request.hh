#ifndef IOFWD_REQUEST_HH
#define IOFWD_REQUEST_HH


namespace iofwd
{
//===========================================================================

/**
 * This class represents a request from a client. It contains the necessary
 * information to reply to the client.
 *
 *
 */
class Request 
{
public:

   Request (int opid); 

   // Return the ZoidFS operation associated with this request 
   int getOpID () const
   {
      return opid_; 
   }; 

   virtual void setStatus (int status)
   {
      status_ = status; 
   }

   // Called in the worker thread when the request gets some cputime
   // Needs to return false if all work is done and the request can be
   // destroyed
   virtual bool run () = 0; 

   // Fast requests can possibly take a shortcut and be serviced in the main
   // receiving thread; Note that a request that needs significat time to
   // determine if it is fast or not cannot be fast.
   virtual bool isFast () 
   { return false; }


   const char * opid2Name (int opid) const; 

   const char * opid2Name () const
   { return opid2Name (opid_); }

   virtual ~Request (); 
 
protected:
   // Operation
   int opid_; 

   // Return code
   int status_; 
}; 

//===========================================================================
}

#endif
