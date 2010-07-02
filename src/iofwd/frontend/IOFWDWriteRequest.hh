#ifndef IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/WriteRequest.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"
#include "iofwdutil/HybridAllocator.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDWriteRequest
   : public IOFWDRequest,
     public WriteRequest,
     public iofwdutil::InjectPool<IOFWDWriteRequest>
{
  protected:
     // shared state
     iofwdutil::transform::GenericTransform * GenTransform;

     // variables start with a lowercase letter; Instance variables end with
     // '_'
     iofwdevent::CBType *  userCB_;

     bool op_hint_compress_enabled;
     bool op_hint_headstuff_enabled;

     // non pipelined state
     size_t compressed_size;

     // pipelined state
     volatile int user_callbacks;
     char **compressed_mem;
     volatile int compressed_mem_count;
     volatile int compressed_mem_consume;
     volatile int num_input_bufs;

     typedef struct _buf
     {
	char *buf;
	int byte_count;
     }buf;
     buf *transform_mem;
     volatile int transform_mem_count;
     volatile int transform_mem_consume;

     pthread_mutex_t imp;
     pthread_cond_t icv;
     pthread_mutex_t omp;
     pthread_cond_t ocv;

public:
   IOFWDWriteRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
     : IOFWDRequest (info,res), WriteRequest (opid)
   {
   }

   virtual ~IOFWDWriteRequest ();

   virtual ReqParam & decodeParam ();

   virtual void reply(const CBType & cb);

   // for normal mode
   virtual void recvBuffers(const CBType & cb, RetrievedBuffer * rb);
   virtual void recvComplete(int recvStatus);

   // for pipeline mode
   virtual void recvPipelineBufferCB(iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size);
   virtual void recvPipelineComplete(int recvStatus);
   virtual void dummyPipelineComplete(int recvStatus);

   virtual void initRequestParams(ReqParam & p, void * bufferMem);

   virtual void allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb);
   virtual void releaseBuffer(RetrievedBuffer * rb);
private:
   ReqParam param_;

   iofwdutil::HybridAllocator<4096> h;
   zoidfs::zoidfs_handle_t handle_;
};

//===========================================================================
   }
}

#endif
