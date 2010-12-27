#include "ServiceManager.hh"
#include "Service.hh"

#include "iofwdutil/IOFWDLog.hh"

#include <boost/format.hpp>

using boost::format;

namespace iofwd
{
   namespace service
   {
      //=====================================================================

      ServiceManager::ServiceManager ()
         : log_ (iofwdutil::IOFWDLog::getSource ("servicemanager"))
      {
         ZLOG_DEBUG(log_, "ServiceManager starting...");
      }

      boost::shared_ptr<Service> ServiceManager::lookupService (const
            std::string & name)
      {
         ZLOG_DEBUG(log_, format("service %s requested: starting load...")
               % name);
         {
            boost::mutex::scoped_lock l(lock_);
            ServiceMap::iterator I = running_.find (name);
            if (I != running_.end())
            {
               boost::shared_ptr<Service> ptr (I->second.lock ());
               if (ptr)
               {
                  ZLOG_DEBUG(log_, format("service %s was already running...")
                        % name);
                  return ptr;
               }
               // Service was removed in the mean time; recreate entry
               ZLOG_DEBUG(log_, format("Service %s needs to be restarted...")
                     % name);
            }
         }

         ZLOG_DEBUG(log_, format("creating service %s...") % name);

         boost::shared_ptr<Service> srv (ServiceFactory::construct (name)(*this));

         boost::mutex::scoped_lock l(lock_);
         running_[name] = srv;

         ZLOG_DEBUG(log_, format("service %s loaded...") % name);
         return srv;
      }

      void ServiceManager::registerService (const std::string & name,
            ServiceFunc func)
      {
         boost::mutex::scoped_lock l(lock_);
         ServiceFactory::instance ().add (name, func);
      }

      ServiceManager::~ServiceManager ()
      {
         boost::mutex::scoped_lock l(lock_);
         ServiceMap::iterator I = running_.begin ();
         while (I != running_.end())
         {
            boost::shared_ptr<Service> ptr (I->second.lock());
            if (ptr)
            {
               ZLOG_ERROR(log_, format("ServiceManager shutdown while "
                     "service '%s' is still running!") % I->first);
            }
            ++I;
         }
      }

      void ServiceManager::setParam (const std::string & s,
            const std::string & val)
      {
         params_[s] = val;
      }

      std::string ServiceManager::getParam (const std::string & s)
      {
         return params_[s];
      }

      //=====================================================================
   }
}
