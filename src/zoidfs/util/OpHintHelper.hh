#ifndef IOFWDUTIL_OPHINTHELPER_HH
#define IOFWDUTIL_OPHINTHELPER_HH

#include "zoidfs-xdr.hh"
#include "zoidfs/zoidfs-proto.h"
#include "zoidfs/hints/zoidfs-hints.h"
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
      OpHintHelper(const zoidfs::zoidfs_op_hint_t * op_hint)
         : readonly_(true)
      {
            zoidfs::hints::zoidfs_hint_create(&op_hint_);

            /* deep copy the hint */
            zoidfs::hints::zoidfs_hint_dup(*op_hint, &op_hint_);
      }

      OpHintHelper(zoidfs::zoidfs_op_hint_t * op_hint)
         : readonly_(false)
      {
            zoidfs::hints::zoidfs_hint_create(&op_hint_);

            /* shallow copy the hint */
            zoidfs::hints::zoidfs_hint_copy(op_hint, &op_hint_);
      }

      ~OpHintHelper()
      {
          zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }


   public:
      zoidfs::zoidfs_op_hint_t op_hint_;
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
      int size = 0;
      zoidfs::zoidfs_null_param_t valid_hint;

      zoidfs::hints::zoidfs_hint_get_nkeys(h.op_hint_, &size);
      valid_hint = (size > 0) ? 1 : 0;
      process (p, valid_hint);

      /* if this is a valid hint, encode the elements in the hint list */
      if (valid_hint)
      {
         int i = 0;

         process (p, size);
         assert(size > 0); /* if this is a valid hint, it must have 1 or more hints */

         for(i = 0 ; i < size ; i++)
         {
            int key_len = 0;
            char * key = NULL;
            int value_len = 0;
            char * value = NULL;
            int flag = 0;

            zoidfs::hints::zoidfs_hint_get_nthkeylen(h.op_hint_, i, &key_len);
            key = (char *)malloc(key_len);
            zoidfs::hints::zoidfs_hint_get_nthkey(h.op_hint_, i, key);
            zoidfs::hints::zoidfs_hint_get_valuelen(h.op_hint_, key, &value_len, &flag);
            value = (char *)malloc(value_len);
            zoidfs::hints::zoidfs_hint_get(h.op_hint_, key, value_len, value, &flag);

            process (p, key_len);
            process (p, EncString(key, key_len));
            process (p, value_len);
            process (p, EncString(value, value_len));

            free(key);
            free(value);
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

         /* get the number of hints */
         process (p, size);
         assert(size > 0); /* if this is a valid hint, it must have 1 or more hints */

         /* for each element in the list */
         for(i = 0 ; i < size ; i++)
         {
            int key_len = 0;
            char * key = NULL;
            int value_len = 0;
            char * value = NULL;

            /* decode the hint data from the xdr stream */
            process (p, key_len);
            key = (char *)malloc((sizeof(char) * key_len) + 1);
            process (p, EncString(key, key_len));
            process (p, value_len);
            value = (char *)malloc((sizeof(char) * value_len) + 1);
            process (p, EncString(value, value_len));

            /* add the hint */
            zoidfs::hints::zoidfs_hint_set(h.op_hint_, key, value, value_len);

            /* cleanup local mem allocs */
            free(key);
            free(value);
         }
      }
      /* else, set the hint list to NULL */
      else
      {
        zoidfs::hints::zoidfs_hint_free(const_cast<zoidfs::zoidfs_op_hint_t *>(&(h.op_hint_)));
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
      int size = 0;
      // As far the size calculation is concerned: we should really have
      // know if the size calculation is for reads or writes...
      // for now, just factor in both path components
      zoidfs::hints::zoidfs_hint_get_nkeys(h.op_hint_, &size);
      valid_hint = (size > 0) ? 1 : 0;
      process (p, valid_hint);
      if(valid_hint)
      {
         int i = 0;

         process (p, size);
         assert(size > 0); /* if this is a valid hint, it must have 1 or more hints */

         /* for each element in the list */
         for(i = 0 ; i < size ; i++)
         {
            int key_len = 0;
            char * key = NULL;
            int value_len = 0;
            char * value = NULL;
            int flag = 0;

            zoidfs::hints::zoidfs_hint_get_nthkeylen(h.op_hint_, i, &key_len);
            key = (char *)malloc(key_len);
            zoidfs::hints::zoidfs_hint_get_nthkey(h.op_hint_, i, key);
            zoidfs::hints::zoidfs_hint_get_valuelen(h.op_hint_, key, &value_len, &flag);
            value = (char *)malloc(value_len);
            zoidfs::hints::zoidfs_hint_get(h.op_hint_, key, value_len, value, &flag);

            process (p, key_len);
            process (p, EncString(key, key_len));
            process (p, value_len);
            process (p, EncString(value, value_len));

            /* cleanup */
            free(key);
            free(value);
         }
      }
   }


//===========================================================================
}




#endif
