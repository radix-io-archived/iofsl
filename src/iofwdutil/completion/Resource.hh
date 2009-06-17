#ifndef IOFWDUTIL_COMPLETION_RESOURCE_HH
#define IOFWDUTIL_COMPLETION_RESOURCE_HH

#include <vector>
#include "CompletionID.hh"

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

class ContextBase; 

class Resource
{
public:

   /// Returns true if there are any requests to be completed 
   virtual bool isActive () const =0; 

   /// Returns vector of completed operations ; will append
   virtual void waitAny (std::vector<CompletionID *> & completed) =0; 

   /// Test for completion
   virtual void testAny (std::vector<CompletionID *> & completed, int maxms) =0; 

   /// Test for single item completion
   virtual bool test (CompletionID * id, int maxms) =0; 

   /// Wait for single item
   virtual void wait (CompletionID * id) =0; 


   virtual ~Resource (); 

protected:

   unsigned char getResourceID () const 
   { 
      return resourceid_; 
   }

protected:
   unsigned char resourceid_; 
}; 

//===========================================================================
   }
}
#endif
