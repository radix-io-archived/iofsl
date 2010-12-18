#ifndef ENCODER_SIZE_HH
#define ENCODER_SIZE_HH

#include <algorithm>
#include "iofwdutil/assert.hh"
#include "Util.hh"
#include "Processor.hh"

namespace encoder
{
//===========================================================================

   /**
    * This class indicates how the 'Size' encoders look like.
    * All size calculators must derive from this class.
    */
   struct Size : public Processor
   {
      public:
         Size () : actual_valid_(true),
            actual_size_(0), max_size_(0)
         {
         }

         struct SizeInfo
         {
            SizeInfo () : actual_size_(0), max_size_(0) { }
            SizeInfo (size_t a, size_t b) : actual_size_(a), max_size_(b) { }

            size_t getActualSize () const { return actual_size_; }
            size_t getMaxSize () const { return max_size_; }

            protected:
              size_t actual_size_;
              size_t max_size_;

              friend class Size;

         };

         SizeInfo size () const
         { return SizeInfo (actual_size_, max_size_); }

         /**
          * Return true if the size calculator attempts to calculate the
          * effective encoded size, as opposed to just the maximum size.
          */
         bool isActualValid () const
         { return actual_valid_; }


         /**
          * Enable/disable calculation of actual size,
          * i.e. using the data instead of just the type and maximum size
          * information.
          */
         void calculateActual (bool enable)
         {
            actual_valid_ = enable;
         }

         /**
          * Reset size calculation
          */
         void reset (const SizeInfo & i = SizeInfo())
         {
            actual_size_ = i.actual_size_;
            max_size_ = i.max_size_;
         }

         // -- for use by the process functions --
         void incActualSize (size_t size)
         { actual_size_ += size; }

         void incMaxSize (size_t size)
         { max_size_ += size; }

         void incBoth (size_t size)
         { incActualSize (size); incMaxSize (size); }

      protected:
         bool actual_valid_;
         size_t actual_size_;
         size_t max_size_;
   };

//===========================================================================

   /** HACK:
    *   to avoid having to write the process functions twice (once for const
    *   and once for not-const), we provide a process function which will,
    *   for all Size processors, cast away the constness.
    */
   /*template <typename T, typename P>
   void process (T & enc, const P & val,
         typename only_size_processor<T>::type *  = 0)
   {
      process (enc, const_cast<P &>(val));
   } */

//===========================================================================
}
#endif
