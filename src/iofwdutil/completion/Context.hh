#ifndef IOFWDUTIL_COMPLETION_HH
#define IOFWDUTIL_COMPLETION_HH

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

template <typename T>
class Context : public ContextBase
{
public:


   T & getUser (const CompletionID & id) 
   { }

   void setUser (CompletionID & id)
   {
   }
}; 

//===========================================================================
   }
}

#endif
