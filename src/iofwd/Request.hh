#ifndef IOFWD_REQUEST_HH
#define IOFWD_REQUEST_HH

#include "iofwdevent/CBType.hh"

namespace iofwd
{
//===========================================================================

/**
 * This class represents a request from a client. It encapsulates the necessary
 * information to reply to the client.
 */
class Request
{
public:
   typedef iofwdevent::CBType CBType;

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

   int getReturnCode () const
   { return returncode_; }

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
