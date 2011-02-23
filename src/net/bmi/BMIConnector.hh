#ifndef NET_BMI_BMICONNECTOR_HH
#define NET_BMI_BMICONNECTOR_HH

#include "net/Net.hh"

#include "iofwdutil/IOFWDLog-fwd.hh"
#include "iofwdevent/Resource-fwd.hh"

#include "zoidfs/zoidfs-comm.h"

#include <boost/unordered_map.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>


extern "C" {
#include <bmi.h>
}

namespace net
{
   namespace bmi
   {
      //=====================================================================

      /**
       * Listen using BMI for incoming RPC requests, passes them to registered
       * RPC server.
       */
      class BMIConnector : public Net
      {
         public:
            BMIConnector (iofwdevent::BMIResource & bmi,
                  iofwdutil::IOFWDLogSource & log);

            ~BMIConnector ();

            virtual void lookup (const char * location, AddressPtr * ptr,
                  const iofwdevent::CBType & cb);


            virtual void setAcceptHandler (const AcceptHandler & handler);

            virtual Connection connect (const ConstAddressPtr & addr);


            virtual void createGroup (GroupHandle * group,
               const std::vector<std::string> & members,
               const iofwdevent::CBType & cb);

         protected:

            struct LookupHelper;

            BMI_addr_t getBMIAddr (const ConstAddressPtr & addr) const;

            /// Called when new unexpected messages are available
            void incoming (const iofwdevent::CBException & e);

            /// Post test for unexpected messages
            void postReceive ();

            /// Handles each unexpected message
            void newQuery (const BMI_unexpected_info & i);

            /// Return the tag to use for the specified outgoing connection
            bmi_msg_tag_t getTag (BMI_addr_t dest);

            static void lookupDone (LookupHelper * a,
                  const iofwdevent::CBException & e);

         protected:

            boost::mutex lock_;

            iofwdevent::BMIResource & bmi_;
            iofwdutil::IOFWDLogSource & log_;

            // In principle, the tagspace is unique between a <source,dest>
            // connection (unordered) pair.
            //
            // The difficulty is that if we have an incoming connection from
            // client X with tag T, and we make an outgoing connection to X,
            // in which case we have to ensure that we don't pick tag T.
            //
            // This is solved by assigning a different tag range for outgoing
            // and incoming connections.
            //
            typedef struct
            {
               bmi_msg_tag_t next_outgoing_tag;
               uint32_t      seq_id;
            } OutgoingTagInfo;

            boost::mutex outgoing_tag_lock_;

            typedef boost::unordered_map<BMI_addr_t, OutgoingTagInfo> 
               OutgoingTagType;

            OutgoingTagType outgoing_tag_info_;

            iofwdevent::Handle ue_handle_;

            int incoming_;
            boost::array<BMI_unexpected_info, 16> ue_in_;

            // Complicated way to reserve two RPC_TAG_COUNT tag ranges
            // Note: range is inclusive (i.e. [start, stop])
            enum { RPC_TAG_COUNT = 4000, // number of tags each way
                   RPC_TAG_START = ZOIDFS_BMI_MAXTAG + 1,
                   RPC_TAG_INCOMING_START = RPC_TAG_START,
                   RPC_TAG_INCOMING_STOP =
                      RPC_TAG_INCOMING_START + RPC_TAG_COUNT - 1,
                   RPC_TAG_OUTGOING_START = RPC_TAG_INCOMING_STOP + 1,
                   RPC_TAG_OUTGOING_STOP =
                      RPC_TAG_OUTGOING_START + RPC_TAG_COUNT - 1,
                   RPC_TAG_STOP = RPC_TAG_OUTGOING_STOP };

            enum { RPC_TYPE_QUERY = 0,
               RPC_TYPE_RESPONSE };
            
            // Lock specifically for exec_ to ensure we don't change it while
            // in use
            boost::mutex exec_lock_;
            size_t exec_refcount_;
            AcceptHandler exec_;
      };

      //=====================================================================
   }
}

#endif
