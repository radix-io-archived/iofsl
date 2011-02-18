#ifndef IOFWDCLIENT_CBCLIENT_HH
#define IOFWDCLIENT_CBCLIENT_HH

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/GetAttrClientSM.hh"

#include "zoidfs/zoidfs-async.h"

#include "iofwdutil/IOFWDLog-fwd.hh"

#include "sm/SMManager.hh"
#include "sm/SMClient.hh"

#include <boost/scoped_ptr.hpp>

#include <queue>


namespace iofwdclient
{
   //========================================================================

   /**
    * Implements a callback version of the async zoidfs API
    */
   class CBClient
   {
      public:
         CBClient (bool poll = true);

         ~CBClient ();

      public:

         void cbgetattr (const IOFWDClientCB & cb,
               int * ret,
               const zoidfs::zoidfs_handle_t * handle,
               zoidfs::zoidfs_attr_t * attr,
               zoidfs::zoidfs_op_hint_t * op_hint);


      protected:

        class CBSMWrapper
        {
            public:
                CBSMWrapper(const IOFWDClientCB & cb,
                        sm::SMClient * sm = NULL) :
                    cb_(cb),
                    sm_(sm),
                    wcb_(boost::bind(&CBClient::cbWrapper, this, _1, _2))
                {
                }

                void set(sm::SMClient * sm)
                {
                    sm_ = boost::intrusive_ptr<sm::SMClient>(sm);
                }

                void call(zoidfs::zoidfs_comp_mask_t mask, const
                        iofwdevent::CBException & cbexception)
                {
                    cb_(mask, cbexception);
                }

                const IOFWDClientCB & getWCB()
                {
                    return wcb_;
                }

            protected:
                /* access to CBClient::cbWrapper */
                friend class CBClient;

                const IOFWDClientCB & cb_;
                boost::intrusive_ptr<sm::SMClient> sm_;
                const IOFWDClientCB wcb_;
         };
        
        static void cbWrapper(CBSMWrapper * cbsm,
            zoidfs::zoidfs_comp_mask_t mask,
            const iofwdevent::CBException & cbexception);

         iofwdutil::IOFWDLogSource & log_;
         boost::scoped_ptr<sm::SMManager> smm_;
         bool poll_;
   };

   //========================================================================
}

#endif
