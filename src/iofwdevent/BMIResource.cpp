#include "iofwdutil/tools.hh"
#include "BMIResource.hh"

namespace iofwdevent
{
//===========================================================================

   void BMIResource::start ()
   {
      checkBMI(BMI_open_context (&context_));
      ThreadedResource::start ();
   }

   void BMIResource::stop ()
   {
      ThreadedResource::stop ();
      // somehow, BMI_close_context does not return an error code.
      BMI_close_context (context_);
   }

   BMIResource::~BMIResource ()
   {
   }

   BMIResource::BMIResource ()
      : mempool_ (sizeof(BMIEntry))
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
    * Worker thread: just calls testcontext over and over.
    */
   void BMIResource::threadMain ()
   {
      std::vector<bmi_op_id_t> opids_ (CHECK_COUNT);
      std::vector<bmi_error_code_t> errors_ (CHECK_COUNT);
      std::vector<BMIEntry *> users_ (CHECK_COUNT);
      std::vector<bmi_size_t> sizes_ (CHECK_COUNT);
      
      while (!needShutdown ())
      {
         int outcount;
         checkBMI (BMI_testcontext (opids_.size(), &opids_[0],
                  &outcount, &errors_[0], &sizes_[0],
                  reinterpret_cast<void**>(&users_[0]), WAIT_TIME, context_));

         for (int i=0; i<outcount; ++i)
         {
            BMIEntry & e = *users_[i];

            if (e.actual)
               *e.actual = sizes_[i];

            // completeEntry frees the BMIEntry
            completeEntry (&e, errors_[i]);
         }
      }
   }

//===========================================================================
}
