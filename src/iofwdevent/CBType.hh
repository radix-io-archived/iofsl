#ifndef IOFWDEVENT_CBTYPE_HH
#define IOFWDEVENT_CBTYPE_HH

#include <boost/function.hpp>

namespace iofwdevent
{

   /**
    * This header defines a generic callback format for blocking operations
    * and defines some status codes.
    */

   /**
    * Callback completion indicators.
    *
    * LAST indicates the first free code (and can be used classes such as
    * SingleCompletion or MultiCompletion)
    *
    * Maybe COMPLETED is the only code needed;
    * FAILED is really there to indicate an exception,
    * and CANCELLED could(should?) throw its own exception.
    */
   enum { COMPLETED = 0, CANCELLED, FAILED, LAST };

   typedef boost::function<void (int)> CBType;

   typedef void * Handle;
}


#endif
