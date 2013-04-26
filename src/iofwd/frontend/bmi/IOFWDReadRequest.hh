#ifndef IOFWD_FRONTEND_IOFWDREADREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREADREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/ReadRequest.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class  IOFWDReadRequest
   : public IOFWDRequest,
     public ReadRequest
{
public:
   IOFWDReadRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), ReadRequest (opid)
   {
   }
   virtual ~IOFWDReadRequest ();

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
   boost::scoped_array<bmi_size_t> bmi_mem_sizes;
   bmi_size_t mem_total_size;
};

//===========================================================================
   }
}

#endif
