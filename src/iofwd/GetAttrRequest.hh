#ifndef IOFWD_GETATTRREQUEST_HH
#define IOFWD_GETATTRREQUEST_HH

#include "Request.hh"

namespace iofwd
{

class GetAttrRequest : public Request
{
public:
   typedef struct
   {
     zoidfs::zoidfs_handle_t * handle;
     zoidfs::zoidfs_attr_t * attr;
     zoidfs::zoidfs_op_hint_t * op_hint;
   } ReqParam;

   GetAttrRequest (int opid) 
      : Request (opid)
   {
   }

   virtual const ReqParam & decodeParam () = 0;

   virtual void reply (const CBType & cb, const zoidfs::zoidfs_attr_t * attr) = 0;
};

}

#endif
