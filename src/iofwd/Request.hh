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
