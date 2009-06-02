#ifndef IOFWDUTIL_COMPLETION_RESOURCE_HH
#define IOFWDUTIL_COMPLETION_RESOURCE_HH

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

class Resource
{
public:

   /// Returns true if there are any requests to be completed 
   virtual bool isActive () const = 0; 

   /// Returns vector of completed operations 
   virtual void waitAny (); 

   virtual ~Resource (); 
}; 

//===========================================================================
   }
}
#endif
