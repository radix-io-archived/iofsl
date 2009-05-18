#ifndef CLIENT_COMMCHANNEL_HH
#define CLIENT_COMMCHANELL_HH

#include "iofwdutil/xdr/XDRReader.hh"
#include "iofwdutil/xdr/XDRWriter.hh"
#include "iofwdutil/xdr/XDRSizeProcessor.hh"
#include "iofwdutil/bmi/BMIAddr.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"
#include "iofwdutil/bmi/BMIOp.hh"
#include "iofwdutil/bmi/BMIContext.hh"
#include "iofwdutil/typestorage.hh"
#include "zoidfs/zoidfs-comm.h"

namespace client
{

   /**
    * Utility class for sending/receiving XDR encoded 
    * parameters and bulk data.
    *
    * Also uses BMI alloc for optimal performance
    */
   class CommChannel
   {
      public:
         CommChannel (iofwdutil::bmi::BMIContextPtr context, 
               iofwdutil::bmi::BMIAddr fwdhost);

         ~CommChannel (); 

      public:
         /// Prepare to start a remote I/O call (operation op)
         void prepareOp (int op)
         {
            op_ = op;
            
            sendSizeProcessor_.reset (); 
            
            // add room for operation id
            sendSizeProcessor_ << op_; 

            receiveSizeProcessor_.reset (); 
         }

         /// Return size calculator for request
         iofwdutil::xdr::XDRSizeProcessor & prepareParam ()
         {
            return sendSizeProcessor_; 
         }

         /// Return size calculator for reply 
         iofwdutil::xdr::XDRSizeProcessor & prepareReply ()
         {
            return receiveSizeProcessor_; 
         }

         /// Create the request: should be called only once
         iofwdutil::xdr::XDRWriter & sendParamInit ()
         {
            const size_t needed = sendSizeProcessor_.getSize().actual; 
            request_writer_.reset (buffer_send_.get (needed), needed); 

            // Add operation id
            request_writer_ << op_; 

            return request_writer_; 
         }

         /// If more parameters need to be added
         iofwdutil::xdr::XDRWriter & sendParamCont ()
         { 
            return request_writer_; 
         }

         /// Access the reply
         iofwdutil::xdr::XDRReader & getReply ()
         {
            return request_reader_; 
         }

         /// Execute the request
         void execute ()
         {
            // Make sure the receive buffer is large enough for the reply
            const size_t needed = receiveSizeProcessor_.getSize().actual; 
            buffer_receive_.resize (needed); 
            // post receive
            iofwdutil::bmi::BMIOp receive = bmi_->postReceive (iofwdhost_, 
                  buffer_receive_.get(), buffer_receive_.size (), 
                  buffer_receive_.bmiType(), ZOIDFS_REQUEST_TAG); 
            bmi_->postSendUnexpected (iofwdhost_, 
                  buffer_send_.get(), buffer_send_.size(), 
                  buffer_send_.bmiType(), ZOIDFS_REQUEST_TAG); 
            
            // Wait until 
            size_t UNUSED(received) = receive.wait (); 
            // Reset XDR deserialization
            request_reader_.reset (buffer_receive_.get (needed), needed); 
         }


         // Generic operation suitable for most operations
         template <typename SENDREQ, typename RECEIVEREQ>
         int genericOp (int opid, const SENDREQ & send, const RECEIVEREQ & recv)
         {
            int ret; 

            // Reset XDR streams and indicate operation code
            prepareOp (opid); 

            // Make sure the send buffer is large enough 
            applyTypes (prepareParam(), send); 

            // Init XDR encoder and serialize operation ID
            sendParamInit (); 

            // Serialize the request parameters
            applyTypes (sendParamCont(), send); 

            // All generic operations include a status code as part of the
            // reply
            prepareReply () << ret; 

            // Ensure the buffer for the server's answer is large enough
            applyTypes (prepareReply(), recv); 

            // Execute the send and wait for receive
            execute (); 

            // All generic operations return a status code first 
            getReply () >> ret; 

            // Get the rest of the reply 
            applyTypes (getReply (), recv); 

            return ret; 
         }

    protected:
         // Op we are handling
         int op_; 

         // BMI context to use
         iofwdutil::bmi::BMIContextPtr bmi_; 

         // I/O Forwarding server
         iofwdutil::bmi::BMIAddr iofwdhost_; 

         // Buffer for request send 
         iofwdutil::bmi::BMIBuffer buffer_send_; 

         // Buffer for request reply
         iofwdutil::bmi::BMIBuffer buffer_receive_; 
         
         // Serialization for request
         iofwdutil::xdr::XDRReader request_reader_; 
         iofwdutil::xdr::XDRWriter request_writer_; 

         // Size for send and receive
         iofwdutil::xdr::XDRSizeProcessor sendSizeProcessor_; 
         iofwdutil::xdr::XDRSizeProcessor receiveSizeProcessor_; 

   }; 
}

#endif
