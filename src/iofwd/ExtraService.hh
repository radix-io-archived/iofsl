#ifndef IOFWD_EXTRASERVICE_HH
#define IOFWD_EXTRASERVICE_HH

#include "service/Service.hh"
#include "NestedConfigurable.hh"

namespace iofwd
{
   //========================================================================

   /**
    * This interface defines additional functionality expected from 'extra
    * services' (i.e. 3rd party/independent services such as the FTB service).
    */
   struct ExtraService : public service::Service, public NestedConfigurable
   {
      ExtraService (service::ServiceManager & m);

      virtual ~ExtraService ();
   };

   //========================================================================
}

#endif
