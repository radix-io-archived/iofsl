#ifndef ENCODER_PROCESSOR_HH
#define ENCODER_PROCESSOR_HH

namespace encoder
{
//===========================================================================

   /**
    * All processor types (decode, encode, size) must derive from this class.
    */
   struct Processor
   {

      /**
       * All processor types must implement template <T> operator (const T &
       * t)
       */
      /* template <typename T>
       void operator () (T & t)
       */
   };

//===========================================================================
}

#endif
