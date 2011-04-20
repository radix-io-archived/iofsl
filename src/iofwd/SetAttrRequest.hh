#ifndef IOFWD_SETATTRREQUEST_HH
#define IOFWD_SETATTRREQUEST_HH

#include "Request.hh"

namespace iofwd
{

class SetAttrRequest : public Request
{
public:
   typedef struct
   {
     zoidfs::zoidfs_handle_t * handle;
     zoidfs::zoidfs_sattr_t * sattr;
     zoidfs::zoidfs_attr_t * attr;
     zoidfs::util::ZoidFSOpHint * op_hint;
   } ReqParam;

   SetAttrRequest (int opid) 
      : Request (opid)
   {
   }

   virtual const ReqParam & decodeParam () = 0;
   virtual void reply (const CBType & cb,
         const zoidfs::zoidfs_attr_t * attr) = 0;
}; 

}

#endif
