#ifndef IOFWD_SERVICE_SERVICEEXCEPTION_HH
#define IOFWD_SERVICE_SERVICEEXCEPTION_HH

#include "iofwdutil/ZException.hh"

namespace iofwd
{
   namespace service
   {
      //=====================================================================

      struct ServiceException : public virtual iofwdutil::ZException {};

      struct UnknownServiceException : public virtual ServiceException {};

      typedef boost::error_info<struct tag_service_name,std::string>
         service_name;

      //=====================================================================
   }
}

#endif
