#ifndef IOFWD_FRONTEND_IOFWDREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREQUEST_HH

#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIAddr.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"
#include "iofwdutil/bmi/BMIOp.hh"
#include "iofwdutil/bmi/BMITag.hh"
#include "iofwdutil/bmi/BMIUnexpectedBuffer.hh"

#include "encoder/xdr/XDRReader.hh"
#include "encoder/xdr/XDRWriter.hh"
#include "encoder/xdr/XDRSizeProcessor.hh"
#include "zoidfs/util/FileSpecHelper.hh"
#include "zoidfs/util/OpHintHelper.hh"

#include "zoidfs/util/zoidfs-wrapped.hh"

#include "iofwd/Request.hh"
#include "iofwdutil/typestorage.hh"

#include "iofwdutil/IOFWDLog.hh"
#include "IOFWDResources.hh"

#include "iofwdutil/mm/BMIMemoryManager.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================


/**
 * \brief IOFWDRequest provides common functionality to all the
 *        IOFWDxxxRequest implementations.
 */
class IOFWDRequest
{
public:
   IOFWDRequest (const BMI_unexpected_info & info,
         IOFWDResources & res);

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
      // @TODO: This probably needs to go!
      memset(info.parent_handle.data, 0, sizeof(uint8_t) * 32);
      memset(info.full_path, 0, ZOIDFS_PATH_MAX);
      memset(info.component_name, 0, ZOIDFS_NAME_MAX);

      process(req_reader_, encoder::FileSpecHelper (&info.parent_handle,
              info.component_name, info.full_path));
   }

   void decodeOpHint (zoidfs::zoidfs_op_hint_t ** op_hint)
   {
     process(req_reader_, encoder::OpHintHelper (op_hint));
   }



protected:

   /** 
    * Temporary helper function until we can remove CompletionID related
    * stuff. After it is removed, this can move into simpleReply
    */
   template <typename SENDOP>
   void simpleReplyCommon (const SENDOP & op)
   {
      encoder::xdr::XDRSizeProcessor s;
      applyTypes (s, op);
      /*fprintf (stderr, "simpleReply: actual=%u, max=%u\n",
            s.getSize().actual, s.getSize().max);  */

      // Since we have actual data we can use the actual size as the
      // upper bound for the required memory for the XDR encoding.
      // Note that the actual encoded data size might still be smaller,
      // if not all type encoders return accurate lower bounds on the size.
      beginReply (s.size().getActualSize());
      applyTypes (reply_writer_, op);
   }

   /**
    * \brief Convenience function for simple requests that respond with only
    *        send back one message to the client.
    *
    */
   template <typename SENDOP>
   void simpleReply (const iofwdevent::CBType & cb, const SENDOP & op)
   {
      simpleReplyCommon (op);
      sendReply (cb);
   }

   /**
    * \brief Send full reply is status is OK, only status otherwise
    *
    * Encodes code, and if code == ZFS_OK encodes op.
    * Sends the resulting buffer using sendReply.
    *
    * This function was added to avoid the 
    *   if (getResultCode() == ZFS_OK)
    *       simpleReply (getResultCode << ....)
    *   else
    *       simpleReply (getResultCode << ....)
    *
    */
   template <typename SENDOP>
   void simpleOptReply (const iofwdevent::CBType & cb, int code,
         const SENDOP & op)
   {
      encoder::xdr::XDRSizeProcessor s;

      // We encode ZFS returncodes using int32_t (even though the C prototype
      // type is 'int')
      s << static_cast<int32_t>(code);

      if (code == zoidfs::ZFS_OK)
      {
         applyTypes (s, op);
      }

      beginReply (s.size().getActualSize());

      reply_writer_ << static_cast<int32_t> (code);
      if (code == zoidfs::ZFS_OK)
      {
         applyTypes (reply_writer_, op);
      }

      sendReply (cb);
   }

   /// Start reply of at most maxsize data
   void beginReply (size_t maxsize);

   /// Send the buffer in reply writer
   void sendReply (const iofwdevent::CBType & cb);

protected:

   /// Some general resources such as bmi, timer, ...
   IOFWDResources & r_;

   // BMI connection
   iofwdutil::bmi::BMIContext & bmi_;

   // Memory holding the request
   iofwdutil::bmi::BMIUnexpectedBuffer raw_request_;

   // Where our client is located
   iofwdutil::bmi::BMIAddr addr_;
   iofwdutil::bmi::BMITag  tag_;

   // XDR reader
   encoder::xdr::XDRReader req_reader_;

   // For reply
   encoder::xdr::XDRWriter reply_writer_;

   iofwdutil::bmi::BMIBuffer buffer_send_;
};

//===========================================================================
   }
}

#endif
