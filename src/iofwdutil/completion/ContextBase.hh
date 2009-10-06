#ifndef IOFWDUTIL_COMPLETION_CONTEXTBASE_HH
#define IOFWDUTIL_COMPLETION_CONTEXTBASE_HH


#include <vector>
#include <boost/thread.hpp>
#include "CompletionID.hh"

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

class Resource; 

class ContextBase
{
public:
   void wait (const CompletionID & id, void ** user); 

   bool test (const CompletionID & id, void ** user, unsigned int mstimeout);

   void waitAny (std::vector<CompletionID *> & ret); 

   void testAny (std::vector<CompletionID *> & ret,  unsigned int maxms); 

   bool isActive () const; 

   ~ContextBase (); 

protected:
   unsigned int getActiveCount () const; 

protected:
   friend class Resource; 

   /// Register a new resource with the context
   unsigned char registerResource (Resource * res); 


protected:
   std::vector<Resource *> resources_; 

   mutable boost::mutex lock_; 
}; 

//===========================================================================
   }
}

#endif
