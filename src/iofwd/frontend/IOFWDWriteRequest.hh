#ifndef IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/WriteRequest.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

/**
 * @class IOFWDWriteRequest
 *
 * @brief Handles write request's from the client 
 *
 * Write request handler for the I/O fowarding server. This class takes care of
 * shared operations between the pipeline and non-pipeline write requests 
 * (such as reading a buffer, decoding the request parameters, ect). Actual
 * file writing and other specific pipeline/non-pipeline functions take place 
 * in iofwd/WriteTask.cpp and iofwd/Request.cpp
 *
 */
class IOFWDWriteRequest
   : public IOFWDRequest,
     public WriteRequest
{
public:
   /**
    * Constructor for IOFWDWriteRequest
    * 
    * @param opid Operation ID for the client request 
    * @param info Unexpected message recieved from the client (all initial 
    *             write reuqest's should be unexpected).
    * @param res  Resources availible to handle this request???
    */
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

   // for pipeline mode
   virtual void recvPipelineBufferCB(iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size);

   virtual void initRequestParams(ReqParam & p, void * bufferMem);

   virtual void allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb);
   virtual void releaseBuffer(RetrievedBuffer * rb);
private:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   zoidfs::zoidfs_op_hint_t op_hint_;
   bmi_size_t mem_expected_size;
   bmi_size_t * bmi_mem_sizes;
};

//===========================================================================
   }
}

#endif
