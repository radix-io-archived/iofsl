#ifndef IOFWD_REQUEST_HH
#define IOFWD_REQUEST_HH

namespace iofwd
{
//===========================================================================

/**
 * This class represents a request from a client. It contains the necessary
 * information to reply to the client.
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

   const char * opid2Name (int opid) const; 

   const char * opid2Name () const
   { return opid2Name (opid_); }

   ///
   /// The return code needs to be set before other data is returned
   ///
   void setReturnCode (int ret)
   {
      returncode_ = ret; 
   }

   virtual ~Request (); 

  protected:
   // Operation
   int opid_; 

   /// Return code for the request
   int returncode_; 
}; 

//===========================================================================
}

#endif
