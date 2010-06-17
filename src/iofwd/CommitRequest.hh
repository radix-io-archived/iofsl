#ifndef IOFWD_COMMITREQUEST_HH
#define IOFWD_COMMITREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"

namespace iofwd
{

class CommitRequest : public Request
{
public:

   typedef struct
   {
      zoidfs::zoidfs_handle_t * handle;
      zoidfs::zoidfs_op_hint_t * op_hint;
   } ReqParam;

   CommitRequest (int opid) : Request (opid)
   {
   };

   virtual const ReqParam & decodeParam () = 0;

   virtual void reply (const CBType & cb) = 0; 

};

}

#endif
