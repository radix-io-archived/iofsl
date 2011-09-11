#ifndef IOFWD_TASKSM_TASKSMFACTORY_HH
#define IOFWD_TASKSM_TASKSMFACTORY_HH

#include <boost/function.hpp>
#include "sm/SMManager.hh"
#include "iofwd/Request.hh"
#include "iofwd/tasksm/SharedData.hh"


namespace iofwd
{
   class Request;

   namespace tasksm
   {
      //=======================================================================


      /**
       * This task factory generates state machine tasks
       * @TODO: This is probably not needed for the state machines.
       * Needs to integrated into requesthandler after split of DefRequestHandler is
       * complete
       */
      class TaskSMFactory
      {
         public:

            TaskSMFactory(zoidfs::util::ZoidFSAsync * api,
                  sm::SMManager & smm);

            ~TaskSMFactory();

            void operator () (iofwd::Request * req);

         public:
            SharedData shared_;
      };

      //=======================================================================
   }
}

#endif
