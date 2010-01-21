#include "SimpleSM.hh"

namespace sm
{


   /**
    * A SimpleSM that can be nested; When it leaves it enters it final state,
    * it calls a user specified callback function.
    */
   class SimpleNestedSM : public SimpleSM<T>
   {
      using iofwdevent::CBType;
   public:

      SimpleNestedSM (const CBType & cb)
         : cb_(cb)
      {
      }

      SimpleNestedSM ()
      {
      }

      /**
       * Calling done will call notify the caller of this nested SM
       * that results are available and that it can continue.
       */
      void done (int status)
      {
         if (cb_.empty())
            return;

         cb_(status);
         cb_.clear ();
      }

      /**
       * Set the function that will be called when the this SM completes.
       */
      void setCallback (const CBType & cb)
      {
         cb_ = cb;
      }

      virtual ~SimpleNestedSM ()
      {
         done ();
      }

   protected:
      CBType cb_;

   };


}
