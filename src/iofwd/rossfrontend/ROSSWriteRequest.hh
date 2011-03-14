#ifndef IOFWD_ROSSFRONTEND_ROSSWRITEREQUEST_HH
#define IOFWD_ROSSFRONTEND_ROSSWRITEREQUEST_HH

#include "ROSSRequest.hh"
#include "iofwd/WriteRequest.hh"

namespace iofwd
{
   namespace rossfrontend
   {
class ROSSWriteRequest
   : public ROSSRequest,
     public WriteRequest
{
public:
   ROSSWriteRequest (int opid)
     : ROSSRequest(), WriteRequest (opid)
   {
   }

   virtual ~ROSSWriteRequest();

   virtual ReqParam & decodeParam();

   virtual void reply(const CBType & cb);

   // for normal mode
   virtual void recvBuffers(const CBType & cb, RetrievedBuffer * rb);

   // for pipeline mode
   virtual void recvPipelineBufferCB(iofwdevent::CBType cb,
           RetrievedBuffer * rb, size_t size);

   virtual void initRequestParams(ReqParam & p, void * bufferMem);

   virtual void allocateBuffer(iofwdevent::CBType cb,
           RetrievedBuffer * rb);
   virtual void releaseBuffer(RetrievedBuffer * rb);
private:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   zoidfs::zoidfs_op_hint_t op_hint_;
   zoidfs::zoidfs_file_size_t mem_expected_size;
   zoidfs::zoidfs_file_size_t * mem_sizes;
};

//===========================================================================
   }
}

#endif
