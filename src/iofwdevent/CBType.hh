#ifndef IOFWDEVENT_CBTYPE_HH
#define IOFWDEVENT_CBTYPE_HH

#include <boost/function.hpp>

namespace iofwdevent
{

   /**
    * This header defines a generic callback format for blocking operations
    * and defines some status codes.
    */

   enum { COMPLETED = 0, CANCELLED, FAILED };

   typedef boost::function<void (int)> CBType;

}


#endif
