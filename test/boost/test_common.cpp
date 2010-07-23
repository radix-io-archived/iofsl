#include <cstdlib>
#include <boost/algorithm/string/predicate.hpp>
#include "test_common.hh"

using namespace std;

bool isLongTestEnabled ()
{
   const char * p = std::getenv ("IOFSL_TEST");
   return (p && boost::iequals (p, "long"));
}
