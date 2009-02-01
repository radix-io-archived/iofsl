#include "TimeVal.hh"

namespace iofwdutil
{

std::ostream & operator << (std::ostream & out, const TimeVal & t)
{
   out << t.getFraction() << 's'; 
   return out; 
}

}

