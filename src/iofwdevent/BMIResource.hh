#ifndef IOFWDEVENT_BMIRESOURCE_HH
#define IOFWDEVENT_BMIRESOURCE_HH

extern "C" 
{
#include <bmi.h>
}

#include <boost/pool/pool.hpp>
#include <deque>

#include "ThreadedResource.hh"
#include "ResourceOp.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/IOFWDLog.hh"

namespace iofwdevent
{
   //===========================================================================

   /**
    * @TODO: consider adding cancel: make operations return some opaque value
    * (pointer to internal entry) and provide a cancel method.
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

      protected:
         // Note: don't store objects in here, mempool doesn't
         // destruct/construct properly
         struct BMIEntry 
         {
            ResourceOp * op;
            bmi_size_t * actual;
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
          * (which will be tunneled into the ResourceOp)
          */
         void handleBMIError (ResourceOp * u, int bmiret);

         /**
          * Return new BMI entry 
          */
         inline BMIEntry * newEntry (ResourceOp * u, bmi_size_t * actual = 0);

         /**
          * CHeck for normal BMI errors not associated with requests
          */
         inline void checkBMI (int ret) { ALWAYS_ASSERT(ret >= 0); };

         void completeUnexpectedClient (size_t count);

         void checkUnexpected ();

      public:

         inline void post_send (ResourceOp * u, BMI_addr_t dest,
               const void * buffer,
               bmi_size_t size,
               enum bmi_buffer_type buffer_type,
               bmi_msg_tag_t tag,
               bmi_hint hints);

         inline void post_recv(ResourceOp * u, BMI_addr_t src,
               void *buffer,
               bmi_size_t expected_size,
               bmi_size_t * actual_size,
               enum bmi_buffer_type buffer_type,
               bmi_msg_tag_t tag,
               bmi_hint hints);

         inline void post_send_list(ResourceOp * u, BMI_addr_t dest,
               const void *const *buffer_list,
               const bmi_size_t* size_list,
               int list_count,
               /* "total_size" is the sum of the size list */
               bmi_size_t total_size,
               enum bmi_buffer_type buffer_type,
               bmi_msg_tag_t tag,
               bmi_hint hints);

         inline void post_recv_list(ResourceOp * u,
               BMI_addr_t src,
               void *const *buffer_list,
               const bmi_size_t *size_list,
               int list_count,
               bmi_size_t total_expected_size,
               bmi_size_t * total_actual_size,
               enum bmi_buffer_type buffer_type,
               bmi_msg_tag_t tag,
               bmi_hint hints);

         inline void post_sendunexpected_list(ResourceOp * u,
				 BMI_addr_t dest,
				 const void *const *buffer_list,
				 const bmi_size_t *size_list,
				 int list_count,
				 /* "total_size" is the sum of the size list */
				 bmi_size_t total_size,
				 enum bmi_buffer_type buffer_type,
				 bmi_msg_tag_t tag,
                                 bmi_hint hints);


         inline void post_sendunexpected(ResourceOp * u,
			    BMI_addr_t dest,
			    const void *buffer,
			    bmi_size_t size,
			    enum bmi_buffer_type buffer_type,
			    bmi_msg_tag_t tag,
                            bmi_hint hints);

         /**
          * Post testunexpected call.
          */
         void testunexpected (ResourceOp * u,
               int incount,
               int * outcount,
               BMI_unexpected_info * info);


       protected:
         bmi_context_id context_;

         // Note: not object pool, no destructor/constructor will be called
         boost::pool<> mempool_;

         // Memory for unexpected messages when nobody is accepting them
         boost::pool<> unexpectedpool_;

         struct UnexpectedClient
         {
            ResourceOp * op;
            int   incount;
            int * outcount;
            BMI_unexpected_info * info;

            UnexpectedClient  () {}

            UnexpectedClient (ResourceOp * o, 
                  int in, int * out, BMI_unexpected_info *i)
               : op (o), incount(in), outcount(out), info(i)
            {
            }
         };

         // Lock for the unexpected structures
         boost::mutex ue_lock_;

         std::vector<BMI_unexpected_info> ue_info_;
         std::deque<UnexpectedClient> ue_clientlist_;
         std::queue<BMI_unexpected_info *> ue_ready_;

         iofwdutil::zlog::ZLogSource & log_;

       protected:
         static
         size_t accumulate_helper (size_t other, const UnexpectedClient & in);



   };

   //===========================================================================

   BMIResource::BMIEntry * BMIResource::newEntry (ResourceOp * u, 
         bmi_size_t * actual)
   {
      BMIEntry * e = (BMIEntry *) mempool_.malloc ();
      e->op = u;
      e->actual = actual;
      return e;
   }

   void BMIResource::completeEntry (BMIEntry * e, int bmiret)
   {
      try
      {
         if (bmiret >= 0)
         {
            e->op->success ();
         }
         else
         {
            handleBMIError (e->op, bmiret);
         }
      }
      catch (...)
      {
         ALWAYS_ASSERT(false && "ResourceOp should not throw");
      }
      mempool_.free (e);
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

   void BMIResource::post_send (ResourceOp * u, BMI_addr_t dest,
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


   void BMIResource::post_sendunexpected_list(ResourceOp * u,
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


   void BMIResource::post_sendunexpected(ResourceOp * u,
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


   void BMIResource::post_recv(ResourceOp * u, BMI_addr_t src,
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

   void BMIResource::post_send_list(ResourceOp * u, BMI_addr_t dest,
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

   void BMIResource::post_recv_list(ResourceOp * u,
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
