#ifndef ZOIDFS_FTB_SERVERSELECTOR_HH
#define ZOIDFS_FTB_SERVERSELECTOR_HH

extern "C"
{
#include <bmi.h>
}

#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace zoidfs
{
   //========================================================================

   /**
    * Help decide which forwarder to send a request to.
    *
    * @TODO: fix this when bringing into new client
    */
   class ServerSelector
   {
      public:
         BMI_addr_t getServer ();

         void reportProblem ();

         void addServer (const char * srv);

         void updateServerInfo (const char * src, double load,
               const boost::posix_time::ptime & time, bool add);

      protected:
         boost::mutex lock_;

         struct ServerEntry
         {
            std::string               servername;
            double                    load;
            boost::posix_time::ptime  lastupdate;
         };

         boost::ptr_vector<ServerEntry> servers_;

         BMI_addr_t currentserver_;
   };

   //========================================================================
}

#endif
