#ifndef IOFWDEVENT_BMIRESOURCE_HH
#define IOFWDEVENT_BMIRESOURCE_HH

extern "C"
{
#include <bmi.h>
}

#include "CBType.hh"

#include <boost/intrusive/slist.hpp>
#include <boost/pool/pool_alloc.hpp>
#include "iofwdutil/tools.hh"
#include "ThreadedResource.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/IOFWDLog.hh"

namespace iofwdevent
{
   //==========================================================================

   /**
    * @TODO: Provide cancel support for normal send/receive
    */
   class BMIResource : public ThreadedResource
   {
      public:

         enum {
            // Number of completed message to handle in one call to
            // testcontext
            CHECK_COUNT = 64,

            // How long we wait in testcontext
            WAIT_TIME = 1000,

            // How many unexpected messages we dequeue at once
            UNEXPECTED_SIZE = 64
         };

         BMIResource ();

         virtual ~BMIResource ();

         virtual void start ();

         virtual void stop ();

         virtual bool cancel (Handle h);

      protected:
         // Note: don't store objects in here, mempool doesn't
         // destruct/construct properly
         struct BMIEntry
         {
            BMIEntry (const CBType & c, bmi_size_t * a)
              : cb(c), actual(a)
            { }

            CBType              cb;
            bmi_size_t *        actual;
         };

      protected:

         virtual void threadMain ();

         /**
          * Check if there was a BMI error; If so, deal with it.
          * Check if the operation completed immediately. If so,
          * call the completion callback right away.
          */
         inline void checkBMISendRecv (BMIEntry * e, int bmiret);

         /**
          * Call the correct callback and return the entry back to the mempool.
          */
         inline void completeEntry (BMIEntry * e, int bmiret);

         /**
          * Translate BMI error code into exception
          * (which will be tunneled into the Callback)
          */
         void handleBMIError (const CBType & cb, int bmiret);
         /**
          * Return new BMI entry
          */
         inline BMIEntry * newEntry (const CBType & cb,
               bmi_size_t * actual = 0);

         /**
          * Check for normal BMI errors not associated with requests.
          */
         inline void checkBMI (int ret);

      public:

         inline void get_info(BMI_addr_t dest,
               int option,
               void * inout_param);

         inline void post_send (const CBType &  u, BMI_addr_t dest,
               const void * buffer,
               bmi_size_t size,
               enum bmi_buffer_type buffer_type,
               bmi_msg_tag_t tag,
               bmi_hint hints);

         inline void post_recv(const CBType &  u, BMI_addr_t src,
               void *buffer,
               bmi_size_t expected_size,
               bmi_size_t * actual_size,
               enum bmi_buffer_type buffer_type,
               bmi_msg_tag_t tag,
               bmi_hint hints);

         inline void post_send_list(const CBType &  u, BMI_addr_t dest,
               const void *const *buffer_list,
               const bmi_size_t* size_list,
               int list_count,
               /* "total_size" is the sum of the size list */
               bmi_size_t total_size,
               enum bmi_buffer_type buffer_type,
               bmi_msg_tag_t tag,
               bmi_hint hints);

         inline void post_recv_list(const CBType &  u,
               BMI_addr_t src,
               void *const *buffer_list,
               const bmi_size_t *size_list,
               int list_count,
               bmi_size_t total_expected_size,
               bmi_size_t * total_actual_size,
               enum bmi_buffer_type buffer_type,
               bmi_msg_tag_t tag,
               bmi_hint hints);

         inline void post_sendunexpected_list(const CBType &  u,
				 BMI_addr_t dest,
				 const void *const *buffer_list,
				 const bmi_size_t *size_list,
				 int list_count,
				 /* "total_size" is the sum of the size list */
				 bmi_size_t total_size,
				 enum bmi_buffer_type buffer_type,
				 bmi_msg_tag_t tag,
                                 bmi_hint hints);


         inline void post_sendunexpected(const CBType &  u,
			    BMI_addr_t dest,
			    const void *buffer,
			    bmi_size_t size,
			    enum bmi_buffer_type buffer_type,
			    bmi_msg_tag_t tag,
                            bmi_hint hints);

         /**
          * Post testunexpected call.
          */
         Handle post_testunexpected (const CBType &  u,
               int incount,
               int * outcount,
               BMI_unexpected_info * info);


       protected:
         bmi_context_id context_;

         struct UnexpectedMessage :
            public boost::intrusive::slist_base_hook<>
         {
            UnexpectedMessage (const BMI_unexpected_info & i)
               : info_ (i)
            {
            }

            BMI_unexpected_info info_;
         };

         struct UnexpectedClient :
            public boost::intrusive::slist_base_hook<>
         {
            CBType op;
            int   incount;
            int * outcount;
            BMI_unexpected_info * info;
            size_t sequence;

            UnexpectedClient  () {}

            UnexpectedClient (const CBType &  o,
                  int in, int * out, BMI_unexpected_info *i,
                  size_t seq)
               : op (o), incount(in), outcount(out), info(i),
               sequence(seq)
            {
            }
         };

         struct ue_ready_disposer
         {
            void operator () (UnexpectedMessage * UNUSED(v))
            {
            }
         };

         struct ue_clientlist_disposer
         {
            void operator () (UnexpectedClient * UNUSED(v))
            {
            }
         };

         // Lock for the unexpected structures
         boost::mutex ue_lock_;

         // Intrusive single linked list with support for push_back
         // (cache_last -> true)
         typedef boost::intrusive::slist<UnexpectedClient,
            boost::intrusive::cache_last<true>,
            boost::intrusive::constant_time_size<true> > UEClientListType;

         UEClientListType ue_clientlist_;

         // slist for storing messages until somebody asks for them.
         // We want to take them from BMI, since as long as unexpected
         // messages are present BMI_testcontext returns early.
         boost::intrusive::slist<UnexpectedMessage,
            boost::intrusive::cache_last<true>,
            boost::intrusive::constant_time_size<true> > ue_ready_;

         size_t ue_sequence_;

         boost::pool<> ue_message_pool_;
         boost::pool<> ue_client_pool_;

         iofwdutil::zlog::ZLogSource & log_;

       protected:
         static size_t accumulate_helper (size_t other,
               const UnexpectedClient & in);

         void completeUnexpectedClient (const UnexpectedClient & client);

         void checkUnexpected ();

         void checkNewUEMessages ();

         /// Throw a BMI exception
         void throwBMIError (int ret);
   };

   //===========================================================================
         
   void BMIResource::checkBMI (int ret)
   {
      if (ret >= 0)
         return;

      throwBMIError (ret);
   }

   BMIResource::BMIEntry * BMIResource::newEntry (const CBType &  u,
         bmi_size_t * actual)
   {
      BMIEntry * e = new BMIEntry (u, actual);
      return e;
   }

   void BMIResource::completeEntry (BMIEntry * e, int bmiret)
   {
      CBType cb;
      cb.swap (e->cb);
      delete (e);

      try
      {
         if (bmiret >= 0)
         {
            /* no errors */
            cb (CBException ());
         }
         else
         {
            handleBMIError (cb, bmiret);
         }
      }
      catch (...)
      {
         ALWAYS_ASSERT(false && "Callback should not throw");
      }
   }


   void BMIResource::checkBMISendRecv (BMIEntry * e, int bmiret)
   {
      // If return is 0, the operation is posted.
      if (!bmiret)
         return;

      // Return is != 0, so either immediate completion or error.
      // Either way, we can call the callback now and release the BMIEntry
      // memory.
      completeEntry (e, bmiret);
   }

   void BMIResource::post_send (const CBType &  u, BMI_addr_t dest,
         const void * buffer,
         bmi_size_t size,
         enum bmi_buffer_type buffer_type,
         bmi_msg_tag_t tag,
         bmi_hint hints)
   {
      bmi_op_id_t id;
      BMIEntry * e = newEntry (u);
      checkBMISendRecv (e, BMI_post_send (&id, dest, buffer, size, buffer_type, tag,
               e, context_, hints));
   }


   void BMIResource::post_sendunexpected_list(const CBType &  u,
         BMI_addr_t dest,
         const void *const *buffer_list,
         const bmi_size_t *size_list,
         int list_count,
         /* "total_size" is the sum of the size list */
         bmi_size_t total_size,
         enum bmi_buffer_type buffer_type,
         bmi_msg_tag_t tag,
         bmi_hint hints)
   {
      bmi_op_id_t id;
      BMIEntry * e = newEntry (u);
      checkBMISendRecv (e, BMI_post_sendunexpected_list (&id,
               dest, buffer_list, size_list, list_count,
               total_size, buffer_type, tag, e, context_, hints));
    }


   void BMIResource::post_sendunexpected(const CBType &  u,
         BMI_addr_t dest,
         const void *buffer,
         bmi_size_t size,
         enum bmi_buffer_type buffer_type,
         bmi_msg_tag_t tag,
         bmi_hint hints)
   {
      bmi_op_id_t id;
      BMIEntry * e = newEntry (u);
      checkBMISendRecv (e, BMI_post_sendunexpected (&id, dest, buffer, size, buffer_type, tag,
               e, context_, hints));
}


   void BMIResource::post_recv(const CBType &  u, BMI_addr_t src,
         void *buffer,
         bmi_size_t expected_size,
         bmi_size_t * actual_size,
         enum bmi_buffer_type buffer_type,
         bmi_msg_tag_t tag,
         bmi_hint hints)
   {
      bmi_op_id_t op;
      BMIEntry * e = newEntry (u, actual_size);

      checkBMISendRecv (e, BMI_post_recv (&op, src, buffer, expected_size,
               actual_size, buffer_type, tag, e, context_, hints));
   }

   void BMIResource::get_info(BMI_addr_t dst,
         int option, 
         void * inout_param)
   {
      checkBMI(BMI_get_info(dst, option, inout_param));
   }

   void BMIResource::post_send_list(const CBType &  u, BMI_addr_t dest,
         const void *const *buffer_list,
         const bmi_size_t* size_list,
         int list_count,
         /* "total_size" is the sum of the size list */
         bmi_size_t total_size,
         enum bmi_buffer_type buffer_type,
         bmi_msg_tag_t tag,
         bmi_hint hints)
   {
      bmi_op_id_t op;
      BMIEntry * e = newEntry (u);
      checkBMISendRecv (e, BMI_post_send_list (&op,
               dest, buffer_list, size_list, list_count,
               total_size, buffer_type, tag, e, context_, hints));
   }

   void BMIResource::post_recv_list(const CBType &  u,
         BMI_addr_t src,
         void *const *buffer_list,
         const bmi_size_t *size_list,
         int list_count,
         bmi_size_t total_expected_size,
         bmi_size_t * total_actual_size,
         enum bmi_buffer_type buffer_type,
         bmi_msg_tag_t tag,
         bmi_hint hints)
   {
      bmi_op_id_t op;
      BMIEntry * e = newEntry (u, total_actual_size);
      checkBMISendRecv (e, BMI_post_recv_list(&op, src, buffer_list,
               size_list, list_count, total_expected_size, total_actual_size,
               buffer_type, tag, e, context_, hints));
   }


   //===========================================================================
}

#endif
