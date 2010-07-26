#ifndef IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH

#include <boost/scoped_ptr.hpp>

#include "IOFWDRequest.hh"
#include "iofwd/WriteRequest.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"
#include "iofwdutil/HybridAllocator.hh"
#include "iofwdutil/InjectPool.hh"

#include "iofwdutil/hash/HashFactory.hh"
#include "iofwdutil/hash/SHA1Simple.hh"
#include "iofwdutil/hash/HashAutoRegister.hh"
#include "src/iofwdutil/transform/IOFWDZLib.hh"

#include "iofwdutil/atomics.hh"

using namespace iofwdutil;
using namespace iofwdutil::hash;

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
     boost::scoped_ptr<iofwdutil::transform::GenericTransform> transform_;
     boost::scoped_ptr<HashFunc> hashFunc_;
     char *hash_value_;
     iofwdevent::CBType *  userCB_;
     bool op_hint_crc_enabled_;
     bool op_hint_compress_enabled_;
     bool op_hint_headstuff_enabled_;

     // non pipelined state
     size_t mem_slot_;
     size_t mem_slot_bytes_;
     size_t size_of_stuffed_data_;

     // pipelined state
     char   **compressed_mem_;
     size_t compressed_size_;
     char   *decompressed_mem_;
     atomic<unsigned int> decompressed_size_;
     char   **callback_mem_;
     size_t next_slot_;
     size_t user_callbacks_;
     bmi_size_t *mem_expected_size_;
     size_t pipeline_ops_;
     boost::mutex mp_;

public:
   IOFWDWriteRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
     : IOFWDRequest (info,res), WriteRequest (opid),
       transform_(NULL), hashFunc_(NULL),
       hash_value_(NULL), userCB_(NULL),
       op_hint_crc_enabled_(false),
       op_hint_compress_enabled_(false), op_hint_headstuff_enabled_(false),
       mem_slot_(0), mem_slot_bytes_(0), size_of_stuffed_data_(0),
       compressed_mem_(NULL), compressed_size_(0), decompressed_mem_(NULL),
       decompressed_size_(0), callback_mem_(NULL),
       next_slot_(0), user_callbacks_(0), mem_expected_size_(NULL), pipeline_ops_(0)
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
