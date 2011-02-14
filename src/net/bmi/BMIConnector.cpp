#include <algorithm>

#include "iofwdutil/IOFWDLog.hh"
#include "iofwdevent/BMIResource.hh"

#include "BMIConnector.hh"
#include "BMIInputStream.hh"
#include "BMIOutputStream.hh"
#include "BMIStreamHeaders.hh"

#include "BMIAddress.hh"

namespace net
{
   namespace bmi
   {
      //=====================================================================

      BMIConnector::BMIConnector (iofwdevent::BMIResource & bmi,
            iofwdutil::IOFWDLogSource & log)
         : bmi_ (bmi),
           log_ (log),
           exec_refcount_ (0)
      {
         postReceive ();
      }

      BMI_addr_t BMIConnector::getBMIAddr (const AddressPtr & ptr) const
      {
         const BMIAddress & a = dynamic_cast<const BMIAddress &> (*ptr);
         return a.getAddr ();
      }

      /**
       * When we make an outgoing connection, we send pick a tag in the
       * outgoing tag range, but we expect the other party to respond with
       * expected messages in the incoming tag range.
       */
      Connection BMIConnector::connect (const AddressPtr & destptr)
      {
         BMI_addr_t dest = getBMIAddr (destptr);
         bmi_msg_tag_t newtag = getTag (dest);
         BMIStreamHeader h;
         h.version = 0x0001;
         h.cookie = random ();
         h.packet_size = 4*1024*1024;
         h.flags = 0;
         const bmi_msg_tag_t outtag = newtag;
         const bmi_msg_tag_t intag = outtag - RPC_TAG_OUTGOING_START
            + RPC_TAG_INCOMING_START;
         return Connection (new BMIInputStream (log_, bmi_, dest, intag, h),
               new BMIOutputStream (log_, bmi_, dest, outtag, false, h));
      }

      void BMIConnector::setAcceptHandler (const AcceptHandler & exec)
      {
         // Spin in case we're using exec_ right now...
         do
         {
            boost::mutex::scoped_lock l (exec_lock_);

            if (exec_refcount_)
               continue;

            exec_ = exec;
            break;
         } while (true);
      }


      struct BMIConnector::LookupHelper
      {
         std::string location;
         AddressPtr * ptr;
         iofwdevent::CBType cb;
         BMIConnector * self;
         BMI_addr_t addr;
      };

      void BMIConnector::lookupDone (BMIConnector::LookupHelper * a,
            const iofwdevent::CBException & e)
      {
         boost::scoped_ptr<LookupHelper> data (a);

         // Create new addr if lookup was successful
         if (!e.hasException ())
            *data->ptr = new BMIAddress (a->addr);

         a->cb (e);
      }

      void BMIConnector::lookup (const char * loc, AddressPtr * ptr,
            const iofwdevent::CBType & cb)
      {
         ALWAYS_ASSERT(loc && ptr);
         LookupHelper * a = new LookupHelper ();
         a->location = loc;
         a->ptr = ptr;
         a->cb = cb;
         a->self = this;
         bmi_.lookup (
               boost::bind (&BMIConnector::lookupDone, a, _1),
               a->location, &a->addr);
      }

      void BMIConnector::postReceive ()
      {
         // range is inclusive
         ue_handle_ = bmi_.post_testunexpected (
               boost::bind (&BMIConnector::incoming, this, _1),
               ue_in_.size(), &incoming_, &ue_in_[0],
               std::make_pair<bmi_msg_tag_t,bmi_msg_tag_t>(RPC_TAG_START,
                  RPC_TAG_STOP));
      }

      void BMIConnector::incoming (const iofwdevent::CBException & e)
      {
         /* when we cancel our receive */
         if (e.isCancelled ())
            return;

         {
            boost::mutex::scoped_lock l(exec_lock_);
            if (!exec_)
            {
               ZLOG_WARN (log_, "Ignoring incoming RPC request; No server"
                     " active");
               return;
            }
            ++exec_refcount_;
         }

         // @TODO: will need to do something smarter here, maybe just log
         // error and ignore?
         e.check ();

         for (int i=0; i<incoming_; ++i)
         {
            ALWAYS_ASSERT(ue_in_[i].error_code >= 0);
            newQuery (ue_in_[i]);
            ue_in_[i].buffer = 0;
         }

         {
            boost::mutex::scoped_lock l(exec_lock_);
            ASSERT(exec_refcount_);
            --exec_refcount_;
         }

         // Reregister for unexpected packets
         postReceive ();
      }

      /**
       * This method should be called after incrementing exec_refcount_ so
       * that we can assume query_ will not change during the execution of
       * this function.
       */
      void BMIConnector::newQuery (const BMI_unexpected_info & i)
      {
         // call exec processor
         BMIStreamHeader h;
         h.decode (i.buffer, i.size);

         ASSERT (i.tag >= RPC_TAG_OUTGOING_START
               && i.tag <= RPC_TAG_OUTGOING_STOP);

         AcceptInfo info;
         info.source = new BMIAddress (i.addr);

         const bmi_msg_tag_t intag = i.tag;
         const bmi_msg_tag_t outtag = intag - RPC_TAG_OUTGOING_START +
            RPC_TAG_INCOMING_START;
         info.in = new BMIInputStream (log_, bmi_, i, intag);
         info.out=new BMIOutputStream (log_, bmi_, i.addr, outtag, true, h);

         exec_ (info);
      }

      BMIConnector::~BMIConnector ()
      {
      }

      /**
       * getTag returns a tag from the OUTGOING_START..OUTGOING_STOP range.
       * To avoid tag conflicts, the tag scheme used is as follows:
       *  
       *  A connects to B
       *    1) pick unique tag for connecting to B 
       *    (guaranteed to be unique between all A initiated connections to B)
       *    2) B receives unexpected message in tagrange OUTGOING
       *       Any further data will be received through expected messages
       *       with the same tag value.
       *    3) Any data sent from B to A will be using tag
       *        tag - OUTGOING_START + INCOMING_START, in other words the same
       *        tag but in the INCOMING range
       *        A only receives data through expected messages
       *
       *  The reason for this is that tags can only be unique for A->B and not
       *  for A->B and B->A. By splitting the tag space in 2, one half is used
       *  for data received on A->B connections and one half for data on B->A
       *  connections.
       */
      bmi_msg_tag_t BMIConnector::getTag (BMI_addr_t dest)
      {
         boost::mutex::scoped_lock l (outgoing_tag_lock_);

         OutgoingTagType::iterator I = outgoing_tag_info_.find (dest);

         if (I != outgoing_tag_info_.end ())
         {
            // Have existing tag for this connection
            OutgoingTagInfo & t = I->second;
            bmi_msg_tag_t ret = t.next_outgoing_tag++;
            if (t.next_outgoing_tag >= RPC_TAG_OUTGOING_STOP)
            {
               ++t.seq_id;
               t.next_outgoing_tag = RPC_TAG_OUTGOING_START;
            }
            // @TODO: also encode seq_id_ in header
            return ret;
         }

         // No tag record yet. Add one
         OutgoingTagInfo & t = outgoing_tag_info_[dest];
         t.next_outgoing_tag = RPC_TAG_OUTGOING_START;
         t.seq_id = 0;
         return t.next_outgoing_tag++;
      }
            
      void BMIConnector::createGroup (GroupHandle * UNUSED(group),
               const std::vector<std::string> & UNUSED(members),
               const iofwdevent::CBType & UNUSED(cb))
      {
         ALWAYS_ASSERT(false);
      }

      //=====================================================================
   }
}
