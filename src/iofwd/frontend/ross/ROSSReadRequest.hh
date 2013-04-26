#ifndef ROSS_ROSSFRONTEND_ROSSREADREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSREADREQUEST_HH

#include "ROSSRequest.hh"
#include "iofwd/ReadRequest.hh"

namespace iofwd
{
   namespace rossfrontend
   {
//===========================================================================

class  ROSSReadRequest
   : public ROSSRequest,
     public ReadRequest
{
public:
   ROSSReadRequest (int opid)
      : ROSSRequest(), ReadRequest(opid)
   {
   }
   virtual ~ROSSReadRequest ();

   virtual ReqParam & decodeParam ();

   virtual void reply(const CBType & cb);

   // for normal mode
   virtual void sendBuffers(const iofwdevent::CBType & cb, RetrievedBuffer * rb);

   // for pipeline mode
   virtual void sendPipelineBufferCB(const iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size);

   virtual void initRequestParams(ReqParam & p, void * bufferMem);
   virtual void allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb);
   virtual void releaseBuffer(RetrievedBuffer * rb);

private:
   ReqParam param_;
   zoidfs::zoidfs_handle_t handle_;
   zoidfs::zoidfs_op_hint_t op_hint_;
   zoidfs::zoidfs_file_size_t * mem_sizes;
   zoidfs::zoidfs_file_size_t mem_total_size;
};

//===========================================================================
   }
}

#endif
