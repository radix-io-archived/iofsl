#ifndef ENCODER_RESTORESIZE_HH
#define ENCODER_RESTORESIZE_HH

#include "Size.hh"

namespace encoder
{
   //========================================================================


   /**
    * Helper class:
    *   Saves the size of a compatible size processor when constructed,
    *   restores when leaving scope.
    */
   template <typename T>
   class SizeSaver
   {
      public:
      SizeSaver (T & size)
         : size_(size),
           saved_size_ (size.size())
      {
      }

      ~SizeSaver ()
      {
         size_.reset (saved_size_);
      }

   protected:
      T & size_;
      Size::SizeInfo saved_size_;

   };

   //========================================================================
}


#endif
