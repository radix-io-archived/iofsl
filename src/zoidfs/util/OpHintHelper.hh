#ifndef IOFWDUTIL_OPHINTHELPER_HH
#define IOFWDUTIL_OPHINTHELPER_HH

#include "zoidfs/util/ZoidFSHints.hh"
#include "zoidfs-xdr.hh"
#include "zoidfs/zoidfs-proto.h"
#include "encoder/EncoderWrappers.hh"
#include "encoder/Util.hh"
#include "iofwdutil/assert.hh"

namespace encoder
{
//===========================================================================

   // Helper functions to make encoding/decoding RPC easier

   /// Class that takes care of encoding/decoding handle, component, full
   /// specification
   /// Note: This is only intended to be used for SERIALIZATION;
   /// It will not work with deserialization
   class OpHintHelper
   {
   public:
      OpHintHelper (const zoidfs::zoidfs_op_hint_t ** op_hint)
         : op_hint_(const_cast<zoidfs::zoidfs_op_hint_t **>(op_hint)),
         readonly_(true)
      {
      }

      OpHintHelper (zoidfs::zoidfs_op_hint_t ** op_hint)
         : op_hint_(op_hint),
         readonly_(false)
      {
      }


   public:
      zoidfs::zoidfs_op_hint_t ** op_hint_;
      bool readonly_;
   };

   /**
    * If we have a hint, encode a 1 and store the hint.
    * Otherwise, store 0 and do not store the hint.
    */
   template <typename T>
   inline void process (T & p, const OpHintHelper & h,
         typename only_encoder_processor<T>::type * = 0)
   {
      // TODO: this should probably not be zoidfs_null_param_t
      zoidfs::zoidfs_null_param_t valid_hint;
      valid_hint = (h.op_hint_ != NULL) ? 1 : 0;
      process (p, valid_hint);

      /* if this is a valid hint, encode the elements in the hint list */
      if (valid_hint)
      {
         int size = zoidfs::util::ZoidFSHintNumElements(h.op_hint_);
         int i = 0;

         process (p, size);
         assert(size > 0); /* if this is a valid hint, it must have 1 or more hints */
         *(h.op_hint_) = zoidfs::util::ZoidFSHintInit(size);

         for(i = 0 ; i < size ; i++)
         {
            zoidfs::zoidfs_op_hint_t * cur_hint = zoidfs::util::ZoidFSHintIndex(h.op_hint_, i);
            int key_len = strlen(cur_hint->key) + 1;
            char * key = cur_hint->key;
            int value_len = cur_hint->value_len;
            char * value = cur_hint->value;

            process (p, key_len);
            process (p, EncString(key, key_len));
            process (p, value_len);
            process (p, EncString(value, value_len));
         }
      }
   }

   /**
    * If we have a hint, encode a 1 and store the hint.
    * Otherwise, store 0 and do not store the hint.
    */
   template <typename T>
   inline void process (T & p, const OpHintHelper & h,
         typename only_decoder_processor<T>::type * = 0)
   {
      zoidfs::zoidfs_null_param_t valid_hint;
      ALWAYS_ASSERT(!h.readonly_);
      process (p, valid_hint);
      /* if this a valid hint, build the hint list */
      if (valid_hint)
      {
         int size = 0;
         int i = 0;

         process (p, size);
         assert(size > 0); /* if this is a valid hint, it must have 1 or more hints */
         *(h.op_hint_) = zoidfs::util::ZoidFSHintInit(size);

         /* for each element in the list */
         for(i = 0 ; i < size ; i++)
         {
            int key_len = 0;
            char * key = NULL;
            int value_len = 0;
            char * value = NULL;

            process (p, key_len);
            key = (char *)malloc(sizeof(char) * key_len);
            process (p, EncString(key, key_len));
            process (p, value_len);
            value = (char *)malloc(sizeof(char) * value_len);
            process (p, EncString(value, value_len));
            zoidfs::util::ZoidFSHintAdd(h.op_hint_, key, value, value_len, ZOIDFS_HINTS_ZC);
         }
      }
      /* else, set the hint list to NULL */
      else
      {
         *(h.op_hint_) = NULL;
      }
   }

   /**
    * If we have a hint, encode a 1 and store the hint.
    * Otherwise, store 0 and do not store the hint.
    */
   template <typename T>
   inline void process (T & p, const OpHintHelper & h,
         typename only_size_processor<T>::type * = 0)
   {
      zoidfs::zoidfs_null_param_t valid_hint;
      // As far the size calculation is concerned: we should really have
      // know if the size calculation is for reads or writes...
      // for now, just factor in both path components
      valid_hint = (h.op_hint_ != NULL) ? 1 : 0;
      process (p, valid_hint);
      if(valid_hint)
      {
         int size = zoidfs::util::ZoidFSHintNumElements(h.op_hint_);
         int i = 0;

         process (p, size);
         assert(size > 0); /* if this is a valid hint, it must have 1 or more hints */
         *(h.op_hint_) = zoidfs::util::ZoidFSHintInit(size);

         /* for each element in the list */
         for(i = 0 ; i < size ; i++)
         {
            zoidfs::zoidfs_op_hint_t * cur_hint = zoidfs::util::ZoidFSHintIndex(h.op_hint_, i);
            int key_len = strlen(cur_hint->key) + 1;
            char * key = cur_hint->key;
            int value_len = cur_hint->value_len;
            char * value = cur_hint->value;

            process (p, key_len);
            process (p, EncString(key, key_len));
            process (p, value_len);
            process (p, EncString(value, value_len));
         }
      }
   }


//===========================================================================
}




#endif
