#ifndef ZOIDFS_FTB_SERVERSELECTOR_HH
#define ZOIDFS_FTB_SERVERSELECTOR_HH

#include "iofwdutil/IOFWDLog-fwd.hh"
#include <boost/uuid/uuid.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>

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
            TIME_EXPIRE     = 600,    // Time in seconds before we forget
                                      // about a server
            MAX_ERRORS      = 3,
            MAX_RETRY       = 3

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
         struct ServerEntry;

         // Rate this server (number in [0,1], 0 being best)
         double rankServer (const ServerEntry & e) const;

         // Decide if we switch servers
         bool trySwitch (size_t servercount, double loaddiff) const;

         void calcServer ();

         // Returns true if current server is removed
         bool doExpire ();

         // Get a random boost value
         double getBoost () const;

      protected:
         boost::mutex lock_;

         iofwdutil::IOFWDLogSource & log_;

         struct ServerEntry
         {
            boost::uuids::uuid        uuid;
            std::string               servername;
            double                    load;
            boost::posix_time::ptime  lastupdate;
            boost::posix_time::ptime  lasterror;
            size_t                    errorcount;
            double                    boost;

            double calcServerRank () const;
            ServerEntry ();
         };

         struct ServerCompare : public
                              std::binary_function<ServerEntry,ServerEntry,bool>
         {
            bool operator () (const ServerEntry & e1, const ServerEntry & e2)
               const;
         };

      protected:

         typedef boost::ptr_vector<ServerEntry> ServerList;
         ServerList servers_;

         ServerEntry * current_;
         BMI_addr_t currentserver_;
         int nextaction_;

         mutable boost::mt11213b rnd_;
         mutable boost::uniform_real<double> boostfactor_;
   };

   //========================================================================
}

#endif
