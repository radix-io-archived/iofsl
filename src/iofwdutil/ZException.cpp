#include <sstream>
#include <boost/foreach.hpp>
#include "ZException.hh"

using namespace std; 

namespace iofwdutil
{

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
      BOOST_FOREACH(const std::string & val, msg_)
      {
         o << val << endl; 
      }
      return o.str(); 
   }

   void ZException::pushMsg (const std::string & msg)
   {
      msg_.push_back (msg); 
   }

}
