#ifndef IOFWD_READLINKREQUEST_HH
#define IOFWD_READLINKREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"

namespace iofwd
{
//===========================================================================

class ReadLinkRequest : public Request
{
public:

   typedef struct
   {
      zoidfs::zoidfs_handle_t * handle;
      size_t buffer_length;
      zoidfs::zoidfs_op_hint_t * op_hint;
   } ReqParam;

   ReadLinkRequest (int opid) :
      Request (opid)
   {
   }

   /**
    * Retrieve the request input parameters
    */
   virtual const ReqParam & decodeParam ()  = 0;

   /**
    * Reply with the handle or 0 if an error occurred and the handle does not
    * need to be transmitted
    */
   virtual void reply (const CBType & cb, const char * buffer,
                                            size_t buffer_length) = 0;
};


//===========================================================================
}


#endif
