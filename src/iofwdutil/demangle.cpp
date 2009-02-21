#include <cxxabi.h>
#include <string>
#include "demangle.hh"
#include "iofwdutil/assert.hh"

namespace iofwdutil
{
#if (__GNUC__ && __cplusplus && __GNUC__ >= 3)
std::string demangle(const char* name)
{
    size_t size;
    int status;
    char* res = abi::__cxa_demangle (name,
                                 0,
                                 &size,
                                 &status);
    ALWAYS_ASSERT(res); 
    std::string ret (res); 
    std::free(res); 
    return ret; 
  }
#else
std::string demangle(const char* name)
{
  return std::string(name);
}
#endif


}
