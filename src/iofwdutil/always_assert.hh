#ifndef IOFWDUTIL_ALWAYS_ASSERT_HH
#define IOFWDUTIL_ALWAYS_ASSERT_HH

#define ALWAYS_ASSERT(a) if (!a) iofwdutil::always_assert ((a), __LINE__,\
      __FILE__, #a) 

namespace iofwdutil
{

void always_assert (bool t, unsigned int line, const char * file, const char
      *msg); 

}


#endif
