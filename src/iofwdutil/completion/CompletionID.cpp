#include "CompletionID.hh"
#include "ContextBase.hh"

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

      static unsigned char CompletionID::INVALID = 
         std::numeric_limits<unsigned char>::max();

      void CompletionID::wait ()
      {
         context_.wait (*this); 
      }

      void CompletionID::test (unsigned int mstimeout)
      {
         context_.test (*this, mstimeout); 
      }

void CompletionID::dummy ()
{
}

//===========================================================================
   }
}
