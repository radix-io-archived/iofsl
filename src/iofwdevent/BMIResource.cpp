#include <numeric>
#include <boost/format.hpp>

#include "BMIException.hh"
#include "iofwdutil/tools.hh"
#include "BMIResource.hh"
#include "BMIError.hh"

using namespace boost;

namespace iofwdevent
{
//===========================================================================

   void BMIResource::lookupHelper (const CBType & u, const std::string & s,
         BMI_addr_t * addr)
   {
      try
      {
         checkBMI (BMI_addr_lookup (addr, s.c_str()));
         u (iofwdevent::CBException ());
         return;
      }
      catch (BMIException & e)
      {
         u (CBException::current_exception ());
      }
      catch (const boost::thread_interrupted & e)
      {
         // someone cancelled us
         u (CBException::cancelledOperation (0));
      }
   }

   BMIResource::Handle BMIResource::lookup (const CBType & u, const
         std::string & s, BMI_addr_t * addr)
   {
      // We do the lookup in a separate thread (bmi does not support async
      // lookup)
      // @TODO: Use threadpool or another method of having cancel support?
      boost::thread thr (boost::bind (&BMIResource::lookupHelper, this, u, s,
               addr));
      return 0;
   }

   /**
    * Note: this method may complete before returning if there are unexpected
    * messages ready.
    */
   BMIResource::Handle BMIResource::post_testunexpected (const CBType &  u,
         int incount, int * outcount, BMI_unexpected_info * info,
         boost::optional<bmi_msg_tag_t> tag)
   {
      boost::mutex::scoped_lock l (ue_lock_);

      ALWAYS_ASSERT(incount > 0);

      *outcount = 0;
      // might want to switch to pool allocated if it has performance benefits
      UnexpectedClient * nc = new (ue_client_pool_.malloc())
                               UnexpectedClient (u, incount, outcount, info,
                                     ue_sequence_++, tag);
      ue_clientlist_.push_back (*nc);

      ZLOG_DEBUG_MORE(log_, format("testunexpected posted: incount=%i new"
               " clientlist size: %i client: %p")
            % incount % ue_clientlist_.size() % u);

      // We have the unexpected lock so we can just as well test for
      // unexpected messages
      checkUnexpected ();

      return reinterpret_cast<void*>(static_cast<intptr_t> (nc->sequence));
   }

   /**
    * Cancel for testunexpected is only called when we shut the server down.
    * Therefore, this is not optimized at all.
    */
   bool BMIResource::cancel (Handle h)
   {
      boost::mutex::scoped_lock l(ue_lock_);

      const size_t seq = static_cast<size_t> (reinterpret_cast<intptr_t> (h));

      // TODO: if we allow cancel for other operations, figure out how to see
      // if it is a testunexpected call or not.

      UEClientListType::iterator I = ue_clientlist_.begin ();
      while (I != ue_clientlist_.end())
      {
         if (I->sequence == seq)
         {
            UnexpectedClient * c = & (*I);
            ue_clientlist_.erase (I);
            c->op (CBException::cancelledOperation (h));
            c->~UnexpectedClient ();
            ue_client_pool_.free (c);
            return true;
         }
         I++;
      }
      return false;
   }

   void BMIResource::start ()
   {
      checkBMI(BMI_open_context (&context_));
      ThreadedResource::start ();
      ZLOG_DEBUG(log_,format("BMIResource started... BMI context: %p") % context_);
   }

   void BMIResource::stop ()
   {
      ThreadedResource::stop ();
      // somehow, BMI_close_context does not return an error code.
      BMI_close_context (context_);
      ZLOG_DEBUG(log_,format("BMIResource stopped... BMI context: %p") % context_);
   }

   BMIResource::~BMIResource ()
   {
      // It is an error if they are still outstanding operations when
      // the resource is destroyed.
      //
      // Clients of the resource should unregister/cancel before the resource
      // itself is destroyed.
      //
      // Since we only support cancelling unexpected receives for now,
      // we clear out the lists here.
      //
      // this is a hack.
      //
      // @TODO Fix cancel of BMI resource operations
      //
      ue_ready_.clear_and_dispose(ue_ready_disposer());
      ue_clientlist_.clear_and_dispose(ue_clientlist_disposer());
   }

   BMIResource::BMIResource ()
      : ue_sequence_ (0),
      ue_message_pool_ (sizeof (UnexpectedMessage)),
      ue_client_pool_ (sizeof (UnexpectedClient)),
      log_ (iofwdutil::IOFWDLog::getSource ("bmiresource"))
   {
   }

   void BMIResource::throwBMIError (int ret)
   {
#ifndef NDEBUG
      char buf[500];
      bmi_strerror_r (ret, buf, sizeof(buf));
      ZLOG_INFO(log_, format("BMI error: %s") % buf);
#endif
      ZTHROW (BMIException () << bmi_error_code (ret));
   }

   /**
    * Calls the callback with a BMI exception. Only returns when the callback
    * has completed.
    */
   void BMIResource::handleBMIError (const CBType & u, int bmiret)
   {
      try
      {
         throwBMIError (bmiret);
      }
      catch (BMIException & e)
      {
         u (CBException::current_exception ());
      }
   }

   /**
    * Complete the specified unexpected client
    * Must be called with ue_lock_ held.
    */
   void BMIResource::completeUnexpectedClient (const UnexpectedClient & client)
   {
#ifndef NDEBUG
      try
      {
#endif
         ZLOG_DEBUG_EXTREME(log_,format("unexpected client %p completed") %
               client.op);
         client.op (CBException ());
#ifndef NDEBUG
      }
      catch (...)
      {
         ALWAYS_ASSERT(false && "ResourceOp should not throw!");
      }
#endif
   }

   size_t BMIResource::accumulate_helper (size_t other, const UnexpectedClient
         & in)
   {
      return other + in.incount;
   }


   /**
    * Call BMI_testunexpected and store messages in ue_ready_
    *
    * Needs to be called with ue_lock_ held
    */
   void BMIResource::checkNewUEMessages ()
   {
      boost::array<BMI_unexpected_info, UNEXPECTED_SIZE> ue_info;

      // Check if there are new messages
      int outcount;

      ASSERT(ue_info.size ());

      // Get unexpected messages without waiting.
      checkBMI (BMI_testunexpected (ue_info.size (), &outcount,
               &ue_info[0], 0));

      ZLOG_DEBUG_EXTREME(log_, format("BMI_testunexpected: outcount=%i, "
               "clientlist size=%i existing queue size=%i")
            % outcount % ue_clientlist_.size() % ue_ready_.size());

      if (!outcount)
         return;

      for (int i=0; i<outcount; ++i)
      {
         UnexpectedMessage * e = new (ue_message_pool_.malloc ())
            UnexpectedMessage (ue_info[i]);
         ue_ready_.push_back (*e);
      }
   }


   /**
    * Check for unexpected messages, and hand them to the listeners or store
    * them if nobody is listening right now.
    *
    * Must be called with ue_lock_ held.
    * NOTE: Will release ue_lock_ but acquire it again before returning.
    * NOTE: Is called both from the polling thread and from the client when it
    * calls post_testunexpected
    */
   void BMIResource::checkUnexpected ()
   {
      // If somebody is already in this function, just exit. No point in
      // racing against eachother to call the callbacks.
      boost::mutex::scoped_try_lock ul (ue_dequeue_lock_);
      if (!ul.owns_lock ())
         return;

      // To simplify things, we always queued the unexpected messages first.
      // This way, we don't have to block other calls to BMI_testunexpected
      // (to prevent writing to ue_info_) while we drop the lock and notify
      // the client.
      checkNewUEMessages ();

      // Now, for as long as we have messages to hand out
      // (either new ones, or queued in ue_ready_) _and_ we have clients
      // we didn't check give them to the client to process them.
      UEClientListType::iterator I = ue_clientlist_.begin();

      
      // Note: ue_dequeue_lock_ prevents two threads from being in this
      // method but does not prevent another thread from appending messages to
      // the ue queue or add another client to the ue_clientlist while we drop
      // the ue_lock during the callback. This is OK since for slist since
      // adding does not invalidate iterators.
      // (see http://www.boost.org/doc/libs/1_35_0/doc/html/
      //  boost/intrusive/slist.html#id847393-bb)
      while (I != ue_clientlist_.end () &&
              !ue_ready_.empty())
      {
         UnexpectedClient & c = *I;

         ASSERT(c.incount);

         size_t delivered = 0;

         // Try to deliver messages

         UEReadyType::iterator M = ue_ready_.begin();

         while (M != ue_ready_.end() &&
               delivered < static_cast<size_t>(c.incount))
         {
            UEReadyType::iterator CUR = M;
            ++M;
            UnexpectedMessage & msg = *CUR;

            // Check for specific tag requests
            if (c.matchtag && msg.info_.tag != *c.matchtag)
               continue;

            c.info[delivered] = msg.info_;
            ue_ready_.erase (CUR);
            ++delivered;

            // call destructor; Boost pool
            msg.~UnexpectedMessage ();

            ue_message_pool_.free (&msg);
         }
         
         // If we have messages for this client,
         // deliver them
         if (delivered)
         {
            UEClientListType::iterator removeclient = I;
            ue_clientlist_.erase (removeclient);
            *c.outcount = delivered;

            // We have to release the lock before calling the callback of the
            // client because most likely it will repost a new testunexpected
            // request.
            //
            // This means that:
            //    1) Somebody could come along and put more unexpected
            //       requests in our ue_ready_ queue
            //    2) ue_client_list iterators can be invalidated
            //       if the client reregisters.
            ue_lock_.unlock ();
            try
            {
               // Note: cannot/should not throw!
               completeUnexpectedClient (c);
               c.~UnexpectedClient ();
            }
            catch (...)
            {
               ALWAYS_ASSERT(false && "callback should not throw!");
            }
            ue_lock_.lock ();
            // We need the lock to protect access to the mem pool
            ue_client_pool_.free (&c);
         }

         // Try next client
         ++I;
      }
   }

   /**
    * Worker thread: just calls testcontext over and over.
    */
   void BMIResource::threadMain ()
   {
      std::vector<bmi_op_id_t> opids_ (CHECK_COUNT);
      std::vector<bmi_error_code_t> errors_ (CHECK_COUNT);
      std::vector<BMIEntry *> users_ (CHECK_COUNT);
      std::vector<bmi_size_t> sizes_ (CHECK_COUNT);

      ZLOG_DEBUG(log_, "polling thread started");

      while (!needShutdown ())
      {
         int outcount;

         // Try to deal with unexpected messages; If we cannot get the lock,
         // somebody is currently calling testunexpected and that thread is
         // going to poll testunexpected any moment now.
         {
            boost::mutex::scoped_try_lock ul (ue_lock_);
            // For now, don't try to queue unexpected messages. Only try to
            // retrieve them if we know we have a client.
            // This is TEMPORARY and only here so that we don't steal unexpected
            // messages from other components in the system.
            if (!ue_clientlist_.empty ())
            {
               if (ul.owns_lock ())
                  checkUnexpected ();
            }
         }
         checkBMI (BMI_testcontext (opids_.size(), &opids_[0],
                  &outcount, &errors_[0], &sizes_[0],
                  reinterpret_cast<void**>(&users_[0]), WAIT_TIME, context_));
         ZLOG_DEBUG_MORE(log_, format("BMI_context: outcount=%i") % outcount);

         polling_ = true;
      }

      // We try to poll for the remainder of the time (until maxtimeout)
      time_duration polltime (maxtimeout - get_system_time ());
      size_t remaining = std::max((boost::int64_t) 0, 
                                  polltime.total_milliseconds ());

      // If we did specify a timeout and it already passed, don't try to poll.
      // Otherwise, if the timeout was zero, try to poll once.
      if (!remaining && maxwaitms != 0)
         return;

      poll_unprotected (remaining);

      // Release polling token and wake waiters
      mutex::scoped_lock l (poll_lock_);
      polling_ = false;
      poll_cond_.notify_one ();
   }

   void BMIResource::poll_unprotected (size_t maxwait)
   {
      ASSERT(opids_.size () ==  CHECK_COUNT);
      ASSERT(errors_.size() == CHECK_COUNT);
      ASSERT(users_.size () == CHECK_COUNT);
      ASSERT(sizes_.size () == CHECK_COUNT);

      int outcount;

      // Try to deal with unexpected messages; If we cannot get the lock,
      // somebody is currently calling post_testunexpected and that thread
      // is going to poll testunexpected any moment now.
      {
         boost::mutex::scoped_try_lock ul (ue_lock_);
         if (ul.owns_lock ())
         {
            // For now, don't try to dequeue unexpected messages if we
            // don't have any clients. Only try to retrieve them if we know
            // we have a client.  This is TEMPORARY and only here so that
            // we don't steal unexpected messages from other components in
            // the system.
            if (!ue_clientlist_.empty ())
            {
               checkUnexpected ();
            }
         }
      }

      checkBMI (BMI_testcontext (opids_.size(), &opids_[0],
               &outcount, &errors_[0], &sizes_[0],
               reinterpret_cast<void**>(&users_[0]), maxwait, context_));
      ZLOG_DEBUG_MORE(log_, format("BMI_context: outcount=%i") % outcount);

      for (int i=0; i<outcount; ++i)
      {
         checkBMI (errors_[i]);
         BMIEntry & e = *users_[i];

            if (e.actual)
               *e.actual = sizes_[i];

            // completeEntry frees the BMIEntry
            completeEntry (&e, errors_[i]);
         }
      }

      ZLOG_DEBUG(log_, "Polling thread stopped");
   }

//===========================================================================
}
