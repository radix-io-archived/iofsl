#include <iterator>
#include <sstream>
#include <algorithm>
#include <boost/foreach.hpp>
#include "ZException.hh"
#include "backtrace.hh"

using namespace std; 

namespace iofwdutil
{
   //========================================================================

   namespace {
      struct addendl
      {
         std::string operator () (const std::string & in)
         {
            return in + "\n";
         }
      };
   }

   ZException::ZException ()
   {

   }

   ZException::ZException (const std::string & s)
   {
      pushMsg (s);
   }

   ZException::~ZException ()
   {
   }

   std::string ZException::toString () const
   {
      ostringstream o; 
      std::ostream_iterator<std::string> out(o);
      std::transform (msg_.begin(), msg_.end(), out, addendl());

      return o.str(); 
   }

   void ZException::pushMsg (const std::string & msg)
   {
      msg_.push_back (msg); 
   }

   //========================================================================
}
