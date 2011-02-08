#ifndef IOFWDEVENT_CBTYPE_HH
#define IOFWDEVENT_CBTYPE_HH

#include "CBException.hh"

#include <boost/function.hpp>

namespace iofwdevent
{

   /**
    * This header defines a generic callback format for blocking operations
    * and defines some status codes.
    */

   /* Use default constructor CBException() to signal no exception */
   typedef boost::function<void (CBException e)> CBType;

}


#endif
