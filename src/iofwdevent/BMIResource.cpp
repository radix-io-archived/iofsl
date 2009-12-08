// #include <boost/lambda/lambda.hpp>
// #include <boost/lambda/bind.hpp>
#include <numeric>
// #include <functional>
#include <boost/format.hpp>

#include "iofwdutil/tools.hh"
#include "BMIResource.hh"

using namespace boost;


namespace iofwdevent
{
//===========================================================================

   /**
    * Note: this method may complete before returning if there are unexpected
    * messages ready.
    */
   void BMIResource::testunexpected (ResourceOp * u,
         int incount, int * outcount, BMI_unexpected_info * info)
   {
      boost::mutex::scoped_lock l (ue_lock_);

      *outcount = 0;
      ue_clientlist_.push_back (UnexpectedClient(u, incount, outcount, info));

      ZLOG_DEBUG(log_, format("testunexpected posted: incount=%i new"
               " clientlist size: %i client: %p") 
            % incount % ue_clientlist_.size() % u);

      // if (ue_ready_)
      checkUnexpected ();
   }

   void BMIResource::start ()
   {
      checkBMI(BMI_open_context (&context_));
      ThreadedResource::start ();
      ZLOG_INFO(log_,format("BMIResource started... BMI context: %p") % context_);
   }

   void BMIResource::stop ()
   {
      ThreadedResource::stop ();
      // somehow, BMI_close_context does not return an error code.
      BMI_close_context (context_);
      ZLOG_INFO(log_,format("BMIResource stopped... BMI context: %p") % context_);
   }

   BMIResource::~BMIResource ()
   {
   }

   BMIResource::BMIResource ()
      : mempool_ (sizeof(BMIEntry)),
      unexpectedpool_ (sizeof (BMI_unexpected_info)),
      ue_info_ (UNEXPECTED_SIZE),
      log_ (iofwdutil::IOFWDLog::getSource ("bmiresource"))
   {
   }


   void BMIResource::handleBMIError (ResourceOp * UNUSED(u), int UNUSED(bmiret))
   {
      // TODO: convert into exception and call u->exception
               // TODO: maybe check for cancel?
               // Shouldn't make a difference right now since we don't do
               // cancel.
      ALWAYS_ASSERT(false && "TODO");
   }

   /**
    * Complete the specified unexpected client
    * Must be called with ue_lock_ held.
    */
   void BMIResource::completeUnexpectedClient (size_t pos)
   {
      ALWAYS_ASSERT(pos < ue_clientlist_.size());
      UnexpectedClient & client = ue_clientlist_[pos];
#ifndef NDEBUG
      try
      {
#endif 
         ZLOG_DEBUG_EXTREME(log_,format("unexpected client %p completed") %
               client.op);
         client.op->success ();
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
    * Check for unexpected messages, and hand them to the listeners or store
    * them if nobody is listening right now.
    *
    * Must be called with ue_lock_ held.
    */
   void BMIResource::checkUnexpected ()
   {
      int outcount;

      ASSERT(ue_info_.size ());

      // Get unexpected messages without waiting.
      checkBMI (BMI_testunexpected (ue_info_.size (), &outcount, 
               &ue_info_[0], 0));

      ZLOG_DEBUG(log_, format("BMI_testunexpected: outcount=%i, "
               "clientlist size=%i queued=%i") 
            % outcount % ue_clientlist_.size() % ue_ready_.size());

      // Total messages is the number of queued messages + 
      // number of new messages; I.e. the number of messages
      // we can hand to the clients
      const size_t total_messages = outcount + ue_ready_.size();
      
      if (!total_messages)
         return;

      // Number of messages we can get rid of
      /*const size_t total_capacity = std::accumulate
         (ue_clientlist_.begin(), ue_clientlist_.end(), int(0),
          bind(std::plus<int>(), _1, bind(&UnexpectedClient::incount,
            _2)));
            */
      const size_t total_capacity = std::accumulate (ue_clientlist_.begin(),
            ue_clientlist_.end(), int (0), 
            accumulate_helper);

      // Number of clients completed
      size_t completed = 0;

      // Number of messages dispatched
      size_t todo = std::min(total_messages, total_capacity);

      // Number of used fresh messages
      size_t consumed = 0;

      // Number of messages used
      size_t total_used = 0;

      while (total_used < todo)
      {
         UnexpectedClient & client = ue_clientlist_[completed];
         const size_t thisclient =
            std::min (total_messages - total_used,
                   static_cast<size_t>(client.incount - *client.outcount));
         if (thisclient)
         {
            for (size_t i=0; i<thisclient; ++i)
            {
               BMI_unexpected_info & dest =
                  client.info[*client.outcount];
               ++(*client.outcount);

               ++total_used;

               // Dequeue a message
               if (ue_ready_.size())
               {
                  // We have a queued message first
                  dest = *ue_ready_.front ();
                  unexpectedpool_.free (ue_ready_.front());
                  ue_ready_.pop ();
               }
               else
               {
                  // Dispatch one of the new messages
                  ALWAYS_ASSERT(consumed <= static_cast<size_t>(outcount));
                  dest = ue_info_[consumed];
                  ++consumed;
               }
            }

            completeUnexpectedClient (completed);
            ++completed;
         }
      }

      // Remove completed
      if (completed)
      {
         ue_clientlist_.erase (ue_clientlist_.begin(),
               ue_clientlist_.begin() + completed);
      }


      // Check if we need to overflow messages
      ALWAYS_ASSERT(consumed <= static_cast<size_t>(outcount));

      if (static_cast<size_t> (consumed)
            == static_cast<size_t>(outcount))
         return;

      const size_t tostore = total_messages - total_capacity;
      ALWAYS_ASSERT(tostore == (outcount - consumed));
      ALWAYS_ASSERT(total_used + tostore == total_messages);

      for (size_t i=0; i<tostore; ++i)
      {
         // Copy unexpected to newly allocated memory pool entry
         BMI_unexpected_info * n =
            (BMI_unexpected_info *) unexpectedpool_.malloc();
         *n = ue_info_[consumed++];
         ue_ready_.push (n);
      }

      ALWAYS_ASSERT(consumed == static_cast<size_t>(outcount));
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

      ZLOG_INFO(log_, "polling thread started");
      
      while (!needShutdown ())
      {
         int outcount;

         // Try to deal with unexpected messages; If we cannot get the lock,
         // somebody is currently calling testunexpected and that thread is
         // going to poll testunexpected any moment now.
         {
            boost::mutex::scoped_try_lock ul (ue_lock_);
            if (ul.owns_lock ())
               checkUnexpected ();
         }

         checkBMI (BMI_testcontext (opids_.size(), &opids_[0],
                  &outcount, &errors_[0], &sizes_[0],
                  reinterpret_cast<void**>(&users_[0]), WAIT_TIME, context_));
         ZLOG_DEBUG(log_, format("BMI_context: outcount=%i") % outcount);

         for (int i=0; i<outcount; ++i)
         {
            BMIEntry & e = *users_[i];

            if (e.actual)
               *e.actual = sizes_[i];

            // completeEntry frees the BMIEntry
            completeEntry (&e, errors_[i]);
         }
      }

      ZLOG_INFO(log_, "Polling thread stopped");
   }

//===========================================================================
}
