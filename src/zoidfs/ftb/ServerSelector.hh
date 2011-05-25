#ifndef ZOIDFS_FTB_SERVERSELECTOR_HH
#define ZOIDFS_FTB_SERVERSELECTOR_HH

#include "iofwdutil/IOFWDLog-fwd.hh"
#include <boost/uuid/uuid.hpp>
#include <boost/random/mersenne_twister.hpp>

extern "C"
{
#include <bmi.h>
}

#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <functional>

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
         enum {
            ACTION_RETURN     = 1,    // return error
            ACTION_RETRY,             // Retry immediately
            ACTION_WAITRETRY,         // Wait and retry
            ACTION_NEWSERVER          // Ask for a new server and retry
         };

         enum
         {
            TIME_EXPIRE     = 600     // Time in seconds before we forget
                                      // about a server

         };

         ServerSelector ();

         BMI_addr_t getServer ();

         // Report problem and returns action to be taken
         int reportProblem ();

         // Return action
         int getAction ();

         void addServer (const char * srv);

         void updateServerInfo (const boost::uuids::uuid & u,
               const char * src, double load,
               bool add = true);

      protected:

         // Decide if we switch servers
         bool trySwitch (size_t servercount, double loaddiff) const;

         void calcServer ();

         // Returns true if current server is removed
         bool doExpire ();

      protected:
         boost::mutex lock_;

         iofwdutil::IOFWDLogSource & log_;

         struct ServerEntry
         {
            boost::uuids::uuid        uuid;
            std::string               servername;
            double                    load;
            boost::posix_time::ptime  lastupdate;
         };

         struct LoadCompare : public
                              std::binary_function<ServerEntry,ServerEntry,bool>
         {
            bool operator () (const ServerEntry & e1, const ServerEntry & e2)
               const
            { return e1.load < e2.load; }
         };

      protected:

         typedef boost::ptr_vector<ServerEntry> ServerList;
         ServerList servers_;

         ServerEntry * current_;
         BMI_addr_t currentserver_;
         int nextaction_;

         mutable boost::mt11213b rnd_;
   };

   //========================================================================
}

#endif
