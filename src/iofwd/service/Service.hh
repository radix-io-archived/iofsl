#ifndef IOFWD_SERVICE_SERVICE_HH
#define IOFWD_SERVICE_SERVICE_HH

#include "iofwdutil/FactoryClient.hh"
#include "ServiceManager.hh"

#include "iofwdutil/LinkHelper.hh"

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <string>

#include <boost/shared_ptr.hpp>

/**
 * This macro needs to be placed in the cpp file of the service,
 * *outside of any namespace*
 *
 * servicename is the name of the service *without quotes*
 */
#define SERVICE_REGISTER(classname,servicename) \
    SERVICE_REGISTER_(classname,#servicename,\
          BOOST_PP_CAT(service_,servicename))

#define SERVICE_REGISTER_(classname,key,servicename) \
   GENERIC_FACTORY_CLIENT(std::string, \
         iofwd::service::Service, \
         classname, \
         key, \
         servicename)



/**
 * The following macro needs to be called with a boost PP sequence to ensure
 * that the services will get registered.
 *
 * Note: call needs to be in a function *outside of any namespace*!
 */

#define SERVICE_LINKHELPER(servicename) \
   GENERIC_FACTORY_LINKHELPER(SERVICE_LINKHELPER_FIXNAME(servicename))

#define SERVICE_LINKHELPER_TRANSFORM(a,b,el) BOOST_PP_CAT(service_,el)

#define SERVICE_LINKHELPER_FIXNAME(services) \
   BOOST_PP_SEQ_TRANSFORM(SERVICE_LINKHELPER_TRANSFORM,~,services)



namespace iofwd
{
   namespace service
   {
   //========================================================================

   /**
    * Base class for a loadable module/service.
    * Takes care of initialization, load order.
    */
   class Service
   {
      public:
         FACTORY_CONSTRUCTOR_PARAMS(ServiceManager &);

         Service (ServiceManager &);

         virtual ~Service ();

      protected:

         /**
          * Returns reference to the requested service.
          * Throws UnknownServiceException if service is not found.
          * Throws bad_cast if the service does not implement the specified
          * interface.
          */
         template <typename INTERFACE>
         inline boost::shared_ptr<INTERFACE> lookupService
                                       (const std::string & name);

         boost::shared_ptr<Service> lookupServiceHelper (const std::string
               & name);

      protected:

         ServiceManager & manager_;
   };

   //========================================================================

   template <typename INTERFACE>
   boost::shared_ptr<INTERFACE> Service::lookupService (const std::string &
         name)
   {
      return manager_.loadService<INTERFACE> (name);
   }

   //========================================================================
   }
}
#endif
