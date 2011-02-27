#ifndef IOFWDCLIENT_CBCLIENT_HH
#define IOFWDCLIENT_CBCLIENT_HH

#include "iofwdclient/iofwdclient-fwd.hh"
#include "iofwdclient/IOFWDClientCB.hh"

#include "zoidfs/zoidfs-async.h"

#include "iofwdutil/IOFWDLog-fwd.hh"
#include "iofwdutil/always_assert.hh"
#include "iofwdutil/tools.hh"

#include "sm/SMManager.hh"
#include "sm/SMClient.hh"

#include <boost/scoped_ptr.hpp>

namespace iofwdclient
{
   //========================================================================

   /**
    * Implements a callback version of the async zoidfs API
    */
   class CBClient
   {
      public:
         CBClient (iofwdutil::IOFWDLogSource & log,
               CommStream & net, bool poll = true);

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
                static CBSMWrapper * createCBSMWrapper(const IOFWDClientCB & cb,
                        sm::SMClient * sm = NULL)
                {
                    return new CBSMWrapper(cb, sm);
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
                /* prevent stack allocation and copying of CBWrapper objects */
                CBSMWrapper(const IOFWDClientCB & cb,
                        sm::SMClient * sm = NULL) :
                    cb_(cb),
                    sm_(sm),
                    wcb_(boost::bind(&CBClient::cbWrapper, this, _1, _2))
                {
                }

                CBSMWrapper() : 
                    cb_(boost::bind(&CBSMWrapper::cbsentinel, this, _1, _2)),
                    sm_(NULL),
                    wcb_(boost::bind(&CBSMWrapper::cbsentinel, this, _1, _2))
                {
                }

                CBSMWrapper(const CBSMWrapper & rhs) :
                    cb_(rhs.cb_),
                    sm_(rhs.sm_),
                    wcb_(rhs.wcb_)
                {
                }

                CBSMWrapper & operator=(const CBSMWrapper & UNUSED(rhs))
                {
                    return *this;
                }

                /* empty callback... should never be invoked */
                void cbsentinel(zoidfs::zoidfs_comp_mask_t UNUSED(mask),
                        const iofwdevent::CBException & UNUSED(cbexception))
                {
                    ALWAYS_ASSERT(false && "CBSMWrapper::cbsentinel was invoked");
                }

                /* access to CBClient::cbWrapper */
                friend class CBClient;

                const IOFWDClientCB & cb_;
                boost::intrusive_ptr<sm::SMClient> sm_;
                const IOFWDClientCB wcb_;
         };
        
        static void cbWrapper(CBSMWrapper * cbsm,
            zoidfs::zoidfs_comp_mask_t mask,
            const iofwdevent::CBException & cbexception);

      protected:
         iofwdutil::IOFWDLogSource & log_;
         CommStream & net_;
         bool poll_;

         boost::scoped_ptr<sm::SMManager> smm_;
   };

   //========================================================================
}

#endif
