#ifndef IOFWDUTIL_COMPLETION_COMPLETIONID_HH
#define IOFWDUTIL_COMPLETION_COMPLETIONID_HH

#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <stdint.h>
#include <algorithm>
#include <limits>

namespace iofwdutil
{
   namespace completion
   {
//=====================================================================

/**
 * This class is supposed to be value copyable. Keep it small.
 * Cannot have virtual functions.
 */
class CompletionID 
{
   public:

      /// Wait until this request completes
      virtual void wait () = 0; 

      /// Test if the request is complete
      virtual bool test (unsigned int mstimeout) = 0; 
      

      void * getUser () const
      { return user_; } 
      
      void setUser (void * user) 
      { user_ = user; }
      
      
      //
      virtual ~CompletionID (); 

protected:

      CompletionID ()
      { }

protected:
      void * user_; 
}; 

//=====================================================================
   }
}

#endif
