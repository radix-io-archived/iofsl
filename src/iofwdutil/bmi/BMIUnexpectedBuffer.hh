#ifndef IOFWDUTIL_BMI_BMIUNEXPECTEDBUFFER_HH
#define IOFWDUTIL_BMI_BMIUNEXPECTEDBUFFER_HH

extern "C"
{
#include <bmi.h>
}

#include "BMIAddr.hh"
#include "BMITag.hh"
#include "iofwdutil/IntrusiveHelper.hh"

namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================

/**
 * This class wraps a BMI_unexpected_info structure and makes sure
 * the memory is released to BMI.
 *
 * TODO: Probably we don't need the IntrusiveHelper anymore.
 */
class BMIUnexpectedBuffer  : public IntrusiveHelper 
{
public:
   /**
    * Note: A copy is made of the BMI_unexpected_info structure
    */
   BMIUnexpectedBuffer (const BMI_unexpected_info & info)
      : info_ (info)
   {
   }

   BMIUnexpectedBuffer () 
   {
      info_.size = 0; 
      info_.buffer = 0; 
   }

   void * get () 
   {
      return info_.buffer; 
   }

   const void * get () const
   { return info_.buffer; }

   size_t size () const 
   { return info_.size; }

   BMIAddr getAddr () const 
   { return BMIAddr (info_.addr); }

   BMITag getTag () const
   { return BMITag (info_.tag); }

   void free ()
   {
      if (info_.buffer)
         BMI_unexpected_free (info_.addr, info_.buffer); 
      info_.buffer = 0; info_.size = 0; 
   }

   ~BMIUnexpectedBuffer ()
   {
      free (); 
   }

private:
   void dummy_ (); 

protected:
   BMI_unexpected_info info_; 
}; 

INTRUSIVE_PTR_HELPER(BMIUnexpectedBuffer)


//===========================================================================
   }
}

#endif
