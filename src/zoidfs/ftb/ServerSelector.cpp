#include "ServerSelector.hh"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/foreach.hpp>
#include "iofwdutil/IOFWDLog.hh"

#include <boost/lambda/lambda.hpp>
#include <boost/format.hpp>

using boost::format;

namespace zoidfs
{
   //========================================================================

   ServerSelector::ServerSelector ()
      : log_ (iofwdutil::IOFWDLog::getSource ("serverselector")),
        current_ (0), nextaction_ (ACTION_RETURN),
        boostfactor_ (0.9,1.1)
   {
   }

   double ServerSelector::getBoost () const
   {
      return boostfactor_ (rnd_);
   }

   BMI_addr_t ServerSelector::getServer ()
   {
      boost::mutex::scoped_lock l (lock_);
      return currentserver_;
   }

   int ServerSelector::getAction ()
   {
      if (nextaction_ != ACTION_RETURN)
      {
         const int oldaction = nextaction_;
         nextaction_ = ACTION_RETURN;
         return oldaction;
      }
      return nextaction_;
   }

   int ServerSelector::reportProblem ()
   {
      boost::mutex::scoped_lock l (lock_);

      ASSERT(current_);
      current_->lasterror = boost::get_system_time ();
      ++current_->errorcount;

      calcServer ();
      return nextaction_;
   }

   ServerSelector::ServerEntry::ServerEntry ()
      : load (0),
        lastupdate (boost::get_system_time ()),
        errorcount (0)
   {
   }

   void ServerSelector::addServer (const char * src)
   {
      boost::mutex::scoped_lock l (lock_);

      const boost::uuids::uuid null = boost::uuids::nil_uuid ();

      // ServerList::iterator I = ;
      ServerEntry * e = new ServerEntry ();
      e->uuid = null;
      e->servername = src;
      e->lastupdate = boost::get_system_time ();
      e->boost = getBoost ();

      calcServer ();
   }

   void ServerSelector::updateServerInfo (const boost::uuids::uuid & u,
         const char * src, double load, bool add)
   {
      boost::posix_time::ptime time = boost::get_system_time ();

      boost::mutex::scoped_lock l (lock_);
      
      ServerEntry * e = 0;

      BOOST_FOREACH (ServerEntry & i, servers_)
      {
         if (i.uuid == u)
         {
            e = &i;
            break;
         }
      }

      if (!e && add)
      {
         e = new ServerEntry ();
         e->uuid = u;
         e->boost = getBoost ();
         servers_.push_back (e);
      }

      e->servername = src;
      e->load = load;
      e->lastupdate = time;

      ZLOG_INFO (log_, format("server %s: rank=%f")
            % e->servername % e->calcServerRank ());

      doExpire ();
      calcServer ();
   }

   double ServerSelector::ServerEntry::calcServerRank () const
   {
      double rank = load;

      rank += errorcount;

      // Scale whole thing by boost
      rank *= boost;

      return rank;
   }
            
   /**
    * This function controls how high a server is ranked in our list
    */
   bool ServerSelector::ServerCompare::operator () (const ServerEntry & e1,
         const ServerEntry & e2) const
   {
      return e1.calcServerRank () < e2.calcServerRank ();
   }

   // Must be called with lock held
   void ServerSelector::calcServer ()
   {
      ASSERT (!servers_.empty ());
      ServerEntry * best = (current_ ? current_ : &servers_.front ());

      if (servers_.size () == 1)
      {
         best = &servers_.front ();
      }
      else
      {
         // pick the best server
         servers_.sort (ServerCompare ());
      }

      if (best != current_)
      {
         double loaddiff = 0;
         if (!current_ || trySwitch (servers_.size(), loaddiff))
         {
            BMI_addr_t newaddr;
            if (BMI_addr_lookup (&newaddr, best->servername.c_str())
                  < 0)
            {
               // Invalid server entry
               // do nothing
               ++best->errorcount;
               best->lasterror = boost::get_system_time ();
            }
            else
            {
               nextaction_ = ACTION_NEWSERVER;
               current_ = best;
               currentserver_ = newaddr;
            }
         }
      }
   }

   bool ServerSelector::trySwitch (size_t , double loaddiff) const
   {
      uint32_t val = (std::min (loaddiff, 1.0) *
            std::numeric_limits<uint32_t>::max());
      // rnd returns random uint32_t
      return (rnd_ () < val);
   }

   // must be called with lock held
   bool ServerSelector::doExpire ()
   {
      // Idea for later: limit size of list, expire oldest entries if size
      // goes over a set limit
      const boost::posix_time::ptime limit =
         boost::get_system_time () - boost::posix_time::seconds(TIME_EXPIRE);

      if (current_ && current_->lastupdate < limit)
         current_ = 0;

      servers_.erase_if (bind (&ServerEntry::lastupdate, _1) < limit);

      return true;
    }

   //========================================================================
}
