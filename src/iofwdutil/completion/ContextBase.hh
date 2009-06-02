#ifndef IOFWDUTIL_COMPLETION_CONTEXTBASE_HH
#define IOFWDUTIL_COMPLETION_CONTEXTBASE_HH


#include "CompletionID.hh"

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

class ContextBase
{
protected:
   friend class CompletionID; 

   void wait (const CompletionID & id); 

   void test (const CompletionID & id, unsigned int mstimeout);

/*   template <typename OUT> 
   unsigned int waitAny (OUT & out);

   template <typename OUT>
   unsigned int testAny (OUT & out); */

   // --- For resources --
   CompletionID createID (unsigned char resourceid, uint32_t privateid)
   { return CompletionID (*this, resourceid, privateid); }

public:

   virtual ~ContextBase (); 
}; 

//===========================================================================
   }
}

#endif
