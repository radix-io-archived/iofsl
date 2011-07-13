#ifndef IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/WriteRequest.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"

#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDWriteRequest
   : public IOFWDRequest,
     public WriteRequest
{
public:
   IOFWDWriteRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
     : IOFWDRequest (info,res), WriteRequest (opid), nbio_enabled_(false)
   {
   }

   virtual ~IOFWDWriteRequest ();

   virtual ReqParam & decodeParam ();

   virtual void reply(const CBType & cb);

   // for normal mode
   virtual void recvBuffers(const CBType & cb, RetrievedBuffer * rb);

   // for pipeline mode
   virtual void recvPipelineBufferCB(iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size);

   virtual void initRequestParams(ReqParam & p, void * bufferMem);

   virtual void allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb);
   virtual void releaseBuffer(RetrievedBuffer * rb);
private:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   zoidfs::util::ZoidFSOpHint op_hint_;
   bmi_size_t mem_expected_size;
   boost::scoped_array<bmi_size_t> bmi_mem_sizes;

   bool nbio_enabled_;
};

//===========================================================================
   }
}

#endif
