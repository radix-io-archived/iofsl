#include "FTBClient.hh"
#include "ServerSelector.hh"

#include "iofwdutil/IOFWDLog.hh"

#include "common/ftb/ftb.pb.h"

#include <arpa/inet.h>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/format.hpp>
#include <ftb.h>

using boost::format;

namespace zoidfs
{
//===========================================================================

      FTBClient::FTBClient (ServerSelector & sel)
         : sel_ (sel),
           log_ (iofwdutil::IOFWDLog::getSource ("ftbclient"))
      {
         FTB_client_t clientinfo;
         strcpy (clientinfo.event_space, "IOFSL.IOFSL.IOFSL");
         strcpy (clientinfo.client_name, "iofslclient");
         strcpy (clientinfo.client_jobid, "blabla");
         strcpy (clientinfo.client_subscription_style,
               "FTB_SUBSCRIPTION_POLLING");
         clientinfo.client_polling_queue_len=0;
         checkFTB (FTB_Connect (&clientinfo, &handle_));


         checkFTB (FTB_Subscribe (&sub_, handle_, "", 0, 0));

         ZLOG_INFO(log_, "FTBClient initialized...");
      }

      void FTBClient::poll ()
      {
         FTB_receive_event_t event;
         const int ret = FTB_Poll_event (sub_, &event);
         if (ret != FTB_SUCCESS)
         {
            if (ret != FTB_GOT_NO_EVENT)
               checkFTB (ret);
            return;
         }
         ZLOG_INFO (log_, format("Received event: %s %s")
               % event.event_space % event.event_name);

         parseData (&event.event_payload[0],
               sizeof(event.event_payload));
      }

      void FTBClient::parseData (const void * p, size_t size)
      {
         ftb::LoadUpdate info;

         const char * ptr = static_cast<const char*>(p);
         size_t used = 0;

         uint32_t len = ntohl (*(const uint32_t*) ptr);
         ptr += sizeof (uint32_t);
         used += sizeof (uint32_t);

         if (used + len >= size)
            return;

         if (!info.ParseFromArray (ptr, len))
         {
            ZLOG_ERROR (log_, format("Could not decode FTB event!"));
            return;
         }

         ZLOG_ERROR(log_, format("Event: %s %f")
               % info.id().location () % info.load());

         boost::uuids::uuid u = boost::uuids::nil_uuid ();
         
         const std::string & uuid = info.id().uuid();
         // invalid uuid
         if (uuid.size() != 16)
            return;

         std::copy (uuid.begin(), uuid.end (), u.begin());

         // Ignore invalid uuids
         if (u.is_nil())
            return;

         sel_.updateServerInfo (u,
               info.id().location ().c_str(), info.load ());
      }

      FTBClient::~FTBClient ()
      {
         checkFTB (FTB_Disconnect (handle_));
      }

      int FTBClient::checkFTB (int ret) const
      {
         if (ret == FTB_SUCCESS)
            return ret;

         ZLOG_ERROR (log_, format("FTB returned error: %i") % ret);
         return ret;
      }

//===========================================================================
}
