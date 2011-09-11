#ifndef IOFWD_SERVICE_SERVICEMANAGER_HH
#define IOFWD_SERVICE_SERVICEMANAGER_HH


#include "iofwdutil/Singleton.hh"
#include "iofwdutil/Factory.hh"
#include "iofwdutil/IOFWDLog-fwd.hh"

#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace iofwd
{
   namespace service
   {
      //=====================================================================

      class Service;

      class ServiceManager : public iofwdutil::Singleton<ServiceManager>
      {
         public:
            typedef Service * (*ServiceFunc) (ServiceManager &);

            ServiceManager ();

            ~ServiceManager ();

            /**
             * Gets a reference to the requested service. Throws
             * UnknownServiceException if service isn't registered or couldn't
             * load.
             */
            boost::shared_ptr<Service> lookupService (const std::string &
                  name);

            /**
             * Helper function that loads the service and tries to cast.
             */
            template <typename T>
            boost::shared_ptr<T> loadService (const std::string & name)
            {
               /*boost::shared_ptr<Service> p (lookupService(name));
               T * casted = dynamic_cast<T*>(p.get());
               if (!casted)
                  throw std::bad_cast ();

               return boost::shared_ptr<T> (p, casted); */
               boost::shared_ptr<T> ptr (
                     boost::dynamic_pointer_cast<T>(lookupService(name)));
               // We know that lookupService doesn't return a 0 pointer, so if
               // ptr is 0 it must be because dynamic_cast failed.
               if (!ptr)
                  throw std::bad_cast ();
               return ptr;
            }

            /**
             * Like loadService, but does not autocreate the service.
             * Returns NULL shared pointer if the service was not yet loaded.
             */
            template <typename T>
            boost::shared_ptr<T> locateService (const std::string & name)
            {
               boost::shared_ptr<T> ptr (
                     boost::dynamic_pointer_cast<T>(lockService(name)));
               return ptr;
            }

            /**
             * Return the service but do not autoload if the service has not
             * been created yet.
             */
            boost::shared_ptr<Service> lockService (const std::string & name);

            /**
             * Register a service with this service manager
             */
            void registerService (const std::string & name, ServiceFunc func);

            /*
             * Startup parameters
             *
             *   This is a communication mechanism providing a method to pass
             *   some parameters to services.
             *
             *   For now, only used to pass the name of the configfile to the
             *   config service.
             */
            void setParam (const std::string & key, const std::string & val);

            /**
             * Returns empty string if key is missing.
             */
            std::string getParam (const std::string & key);

         protected:

            boost::mutex lock_;

            typedef boost::unordered_map<std::string,
                        boost::weak_ptr<Service> > ServiceMap;
            ServiceMap running_;

            typedef iofwdutil::Factory<std::string,Service> ServiceFactory;

            boost::unordered_map<std::string,std::string> params_;

            iofwdutil::IOFWDLogSource & log_;
      };

      //=====================================================================
   }
}
#endif
