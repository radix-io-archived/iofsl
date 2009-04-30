#ifndef IOFWD_FRONTEND_IOFWDREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREQUEST_HH

#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIAddr.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"
#include "iofwdutil/bmi/BMIOp.hh"
#include "iofwdutil/bmi/BMITag.hh"
#include "iofwdutil/bmi/BMIUnexpectedBuffer.hh"
#include "iofwdutil/xdr/XDRReader.hh"
#include "iofwdutil/xdr/XDRWriter.hh"

namespace iofwd
{
   namespace frontend
   {

class IOFWDRequest 
{
public:
   IOFWDRequest (iofwdutil::bmi::BMIContext & bmi, const BMI_unexpected_info & info); 

   /// Release the memory of the incoming request
   void freeRawRequest (); 

   virtual ~IOFWDRequest (); 

   virtual bool run ()
   {
      return false; 
   }

protected:

   /// Needs to be implemented 
   /// virtual void decodeRequest () = 0; 


   /// Actually runs the state machine

protected:
   void beginReply (size_t maxsize);

   iofwdutil::bmi::BMIOp sendReply (); 

protected:
   /// Send a reply back to the client; low-level function
   iofwdutil::bmi::BMIOp ll_sendReply (const void * buf, size_t bufsize,
         bmi_buffer_type); 

protected:
   
   // BMI connection
   iofwdutil::bmi::BMIContext & bmi_; 

   // Memory holding the request
   iofwdutil::bmi::BMIUnexpectedBuffer raw_request_; 

   // Where our client is located
   iofwdutil::bmi::BMIAddr addr_;
   iofwdutil::bmi::BMITag  tag_; 

   // XDR reader
   iofwdutil::xdr::XDRReader req_reader_; 

   // For reply
   iofwdutil::xdr::XDRWriter reply_writer_; 
         
   iofwdutil::bmi::BMIBuffer buffer_send_; 
}; 


   }
}

#endif
