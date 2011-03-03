#include "ServerSelector.hh"

#include <boost/foreach.hpp>

namespace zoidfs
{
   //========================================================================

   BMI_addr_t ServerSelector::getServer ()
   {
      return currentserver_;
   }

   void ServerSelector::reportProblem ()
   {
   }

   void ServerSelector::addServer (const char * src)
   {
      boost::mutex::scoped_lock l (lock_);
      ServerEntry * e = new ServerEntry ();
      e->load = 0;
      e->servername = src;
      e->lastupdate = boost::get_system_time ();
   }

   void ServerSelector::updateServerInfo (const char * src, double load,
         const boost::posix_time::ptime & time, bool add)
   {
      boost::mutex::scoped_lock l (lock_);
      BOOST_FOREACH (ServerEntry & e, servers_)
      {
         if (e.servername == std::string (src))
         {
            e.load = load;
            e.lastupdate = time;
            return;
         }
      }
      if (!add)
         return;

      ServerEntry * e = new ServerEntry ();
      e->load = load;
      e->servername = src;
      e->lastupdate = time;
      servers_.push_back (e);
   }

   //========================================================================
}
