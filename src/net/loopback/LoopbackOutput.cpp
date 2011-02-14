#include "LoopbackOutput.hh"
#include "iofwdutil/assert.hh"

namespace net
{
   namespace loopback
   {
      //=====================================================================

      LoopbackOutput::LoopbackOutput (MessageQueuePtr ptr,
            size_t def_blocksize, size_t max_blocksize)
         : queue_ (ptr), def_blocksize_ (def_blocksize),
           max_blocksize_ (max_blocksize),
           thisblock_ptr_(0), thisblock_size_(0), thisblock_used_(0)
      {
      }

      LoopbackOutput::~LoopbackOutput ()
      {
      }

      iofwdevent::Handle LoopbackOutput::flush (const iofwdevent::CBType & cb)
      {
         internal_flush ();
         cb (iofwdevent::CBException ());
         return 0;
      }

      iofwdevent::Handle LoopbackOutput::write (void ** ptr, size_t * size,
            const iofwdevent::CBType & cb, size_t suggested)
      {
         // If we have some leftover space, use that first
         if (thisblock_used_ < thisblock_size_)
         {
            *ptr = static_cast<char *> (thisblock_ptr_) + thisblock_used_;
            *size = thisblock_size_ - thisblock_used_;

            // Mark everything we gave out as used
            thisblock_used_ += *size;

            cb (iofwdevent::CBException ());
            return 0;
         }

         // put full block in queue
         internal_flush ();

         const size_t newblocksize = suggested ?
                std::min (max_blocksize_, suggested)
              : def_blocksize_;

         // we need to get a new block
         thisblock_ptr_ = new char [newblocksize];
         thisblock_size_ = newblocksize;
         thisblock_used_ = newblocksize;

         *ptr = thisblock_ptr_;
         *size = thisblock_size_;
         cb (iofwdevent::CBException ());
         return 0;
      }

      iofwdevent::Handle LoopbackOutput::rewindOutput (size_t size,
            const iofwdevent::CBType & cb)
      {
         ALWAYS_ASSERT(size <= thisblock_used_);
         thisblock_used_ -= size;
         cb (iofwdevent::CBException ());
         return 0;
      }

      void LoopbackOutput::internal_flush ()
      {
         if (!thisblock_ptr_)
            return;

         queue_->put (thisblock_ptr_, thisblock_used_);
         thisblock_ptr_ = 0;
         thisblock_size_ = 0;
         thisblock_used_ = 0;
      }

      //=====================================================================
   }
}
