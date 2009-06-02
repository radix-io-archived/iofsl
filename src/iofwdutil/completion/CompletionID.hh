#ifndef IOFWDUTIL_COMPLETION_COMPLETIONID_HH
#define IOFWDUTIL_COMPLETION_COMPLETIONID_HH

#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <limits>

namespace iofwdutil
{
   namespace completion
   {
//=====================================================================


using boost::uint16_t; 

class ContextBase; 

/**
 * This class is supposed to be value copyable. Keep it small.
 * Cannot have virtual functions.
 */
class CompletionID 
{
   protected:
      static unsigned char INVALID;

   public:

      // Provide public default constructor for container support
      CompletionID ()
         : context_(0), privateid_(0),
               resourceid_(INVALID); 
protected:
   friend class ContextBase; 

   CompletionID (ContextBase & base, unsigned char res, uint32_t priv)
      : context_(base), privateid_(priv), resourceid_(res)
   {
   }

public:
   void wait ();

   void test (unsigned int mstimout);


   bool operator == (const CompletionID & other) const
   { 
      return 
         boost::addressof(other.context_) == context_ && 
         other.privateid_ == privateid_ && 
         other.resourceid_ == resourceid_ ; 
   } 

private:
   // give the compiler a place to store defs
   void dummy (); 
protected:
   ContextBase * context_; 
   uint32_t      privateid_; 
   unsigned char resourceid_;
}; 

//=====================================================================
   }
}

#endif
