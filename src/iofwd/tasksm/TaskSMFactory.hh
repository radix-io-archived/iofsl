#ifndef IOFWD_TASKSM_TASKSMFACTORY_HH
#define IOFWD_TASKSM_TASKSMFACTORY_HH

#include <boost/function.hpp>
#include "iofwd/BMIMemoryManager.hh"
#include "sm/SMManager.hh"
#include "iofwd/Request.hh"

namespace zoidfs
{
    namespace util
    {
        class ZoidFSAsync;
    }
}

namespace iofwd
{
    class Request;

    namespace tasksm
    {
//===========================================================================


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
         sm::SMManager & smm)
      :  api_(api), smm_(smm)
   {
   }

   ~TaskSMFactory()
   {
   }

   void operator () (iofwd::Request * req);

protected:
   zoidfs::util::ZoidFSAsync * api_;
   sm::SMManager & smm_;
};

    }
//===========================================================================
}

#endif
