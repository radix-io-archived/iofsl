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
#include "iofwdutil/xdr/XDRSizeProcessor.hh"
#include "zoidfs/util/FileSpecHelper.hh"

#include "zoidfs/util/zoidfs-wrapped.hh"

#include "iofwd/Request.hh"
#include "iofwdutil/typestorage.hh"
#include "iofwdutil/completion/CompletionID.hh"

#include "iofwdutil/completion/BMIResource.hh"

#include "iofwdutil/IOFWDLog.hh"

using iofwdutil::completion::CompletionID;

namespace iofwd
{
   namespace frontend
   {

class IOFWDRequest
{
public:
   IOFWDRequest (iofwdutil::bmi::BMIContext & bmi, const BMI_unexpected_info & info,
         iofwdutil::completion::BMIResource & bmires);

   /// Release the memory of the incoming request
   void freeRawRequest ();

   virtual ~IOFWDRequest ();

protected:

   typedef struct
   {
      zoidfs::zoidfs_handle_t parent_handle;
      char full_path[ZOIDFS_PATH_MAX];
      char component_name[ZOIDFS_NAME_MAX];
   } FileInfo;

   // ----------------- decoding/encoding helpers -------------------

   void decodeFileSpec (FileInfo & info)
   {
      memset(info.parent_handle.data, 0, sizeof(uint8_t) * 32);
      memset(info.full_path, 0, ZOIDFS_PATH_MAX);
      memset(info.component_name, 0, ZOIDFS_NAME_MAX);
      process(req_reader_, iofwdutil::xdr::FileSpecHelper (&info.parent_handle,
              info.component_name, info.full_path));
   }



protected:

   // Convenience functions for simple requests (lookup, mkdir, ... )
   template <typename SENDOP>
   CompletionID * simpleReply (const SENDOP & op)
   {
      iofwdutil::xdr::XDRSizeProcessor s;
      applyTypes (s, op);
      /*fprintf (stderr, "simpleReply: actual=%u, max=%u\n",
            s.getSize().actual, s.getSize().max);  */

      // Since we have actual data we can use the actual size as the
      // upper bound for the required memory for the XDR encoding.
      // Note that the actual encoded data size might still be smaller,
      // if not all type encoders return actual lower bounds on the size.
      beginReply (s.getSize().actual);
      applyTypes (reply_writer_, op);

      return sendReply ();
   }

   /// Start reply of at most maxsize data
   void beginReply (size_t maxsize);

   /// Send the buffer in reply writer
   CompletionID * sendReply ();

protected:
   /// Send a reply back to the client; low-level function
   CompletionID * ll_sendReply (const void * buf, size_t bufsize,
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

   iofwdutil::completion::BMIResource & bmires_;

   static iofwdutil::zlog::ZLogSource & log_;
};


   }
}

#endif
