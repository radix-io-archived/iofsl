#ifndef IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH

#include <boost/scoped_ptr.hpp>

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
#define NUM_OUTSTANDING_REQUESTS 8

class IOFWDWriteRequest
   : public IOFWDRequest,
     public WriteRequest,
     public iofwdutil::InjectPool<IOFWDWriteRequest>
{
  protected:
     // shared state
     boost::scoped_ptr<iofwdutil::transform::GenericTransform> transform_;

     // variables start with a lowercase letter; Instance variables end with
     // '_'
     iofwdevent::CBType *  userCB_;

     bool op_hint_compress_enabled;
     bool op_hint_headstuff_enabled;
     size_t mem_slot;
     size_t mem_slot_bytes;
     size_t size_of_stuffed_data;

     // pipelined state
     char **compressed_mem;
     size_t compressed_size;
     char *decompressed_mem;
     size_t decompressedBufSize;
     size_t decompressed_size;
     char **callback_mem;
     int next_slot;

     int user_callbacks;

     boost::mutex mp_;

public:
   IOFWDWriteRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
     : IOFWDRequest (info,res), WriteRequest (opid),
       op_hint_compress_enabled(false), op_hint_headstuff_enabled(false),
       mem_slot(0), mem_slot_bytes(0), size_of_stuffed_data(0),
       compressed_mem(NULL), compressed_size(NULL), decompressed_mem(NULL),
       decompressedBufSize(0), decompressed_size(0), callback_mem(NULL),
       next_slot(0), user_callbacks(0)
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
   virtual void recvPipelineComplete(int recvStatus, int my_slot);

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
