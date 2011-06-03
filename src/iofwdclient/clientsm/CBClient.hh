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
#include "iofwd/RPCClient.hh"
#include "rpc/RPCHandler.hh"
#include <boost/shared_ptr.hpp>

#include <boost/intrusive_ptr.hpp>
#include <cstdio>
/*==========================================================================*/
/* CBClient Creation Macros */
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/seq/cat.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/pop_back.hpp>

/* creates list of input params (aka zoidfs_handle_t * handle,....) */
#define CLIENT_CBCLIENT_PARAMS(r,data,elem) CLIENT_CBCLIENT_PARAMS_S1(elem)
#define CLIENT_CBCLIENT_PARAMS_S1(elem) CLIENT_CBCLIENT_PARAMS_S2(elem),
#define CLIENT_CBCLIENT_PARAMS_S2(elem) BOOST_PP_SEQ_FOLD_LEFT(CLIENT_CBCLIENT_DOFOLD, \
                                        BOOST_PP_SEQ_HEAD(elem),             \
                                        BOOST_PP_SEQ_TAIL(elem))
#define CLIENT_CBCLIENT_DOFOLD(r,data,elem) data elem

/* creates parameter list from PARAMETERS */
#define CLIENT_CBCLIENT_PLIST(r,data,elem) CLIENT_CBCLIENT_PLIST_S1(elem)
#define CLIENT_CBCLIENT_PLIST_S1(elem) CLIENT_CBCLIENT_PLIST_S2(elem),
#define CLIENT_CBCLIENT_PLIST_S2(elem) BOOST_PP_SEQ_CAT(BOOST_PP_SEQ_TAIL(elem))

/* defines the function for the class, takes 3 parameter'. 
   FUNCNAME = the name of the function to be written. 
   SMNAME = Name of the state machine class to use. 
   PARAMETERS = BOOST list of parameters for the function ex: 
      ((zoidfs_handle_t)(handle))((zoidfs_cache_hint_t)(cache)). op_hint is
      to be excluded from this list (it is included by default)
*/
   
#define CLIENT_CBCLIENT_MACRO(FUNCNAME, SMNAME, PARAMETERS)                  \
   void CBClient::FUNCNAME(const IOFWDClientCB & cb,                         \
         int * ret,                                                          \
         BOOST_PP_SEQ_FOR_EACH(CLIENT_CBCLIENT_PARAMS, , PARAMETERS)         \
         zoidfs_op_hint_t * op_hint)                                         \
   {                                                                         \
      CBSMWrapper * cbsm =  CBSMWrapper::createCBSMWrapper(cb);              \
      iofwdclient::clientsm::SMNAME(*smm_, poll_, cbsm->getWCB(), ret,       \
         BOOST_PP_SEQ_FOR_EACH(CLIENT_CBCLIENT_PLIST, , PARAMETERS)          \
         op_hint);                                                           \
      cbsm->set(sm);                                                         \
      sm->execute();                                                         \
   }
/*==========================================================================*/




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
               CommStream & net, net::AddressPtr addr, bool poll = true);

         ~CBClient ();

      public:
//         void setRPCMode ( boost::shared_ptr<iofwd::RPCClient> rpcclient,
//                           net::AddressPtr addr);
         void cbgetattr (const IOFWDClientCB & cb,
               int * ret,
               const zoidfs::zoidfs_handle_t * handle,
               zoidfs::zoidfs_attr_t * attr,
               zoidfs::zoidfs_op_hint_t * op_hint);

         void cbsetattr(const IOFWDClientCB & cb,
                      int * ret,
                      const zoidfs::zoidfs_handle_t *handle,
                      const zoidfs::zoidfs_sattr_t *sattr,
                      zoidfs::zoidfs_attr_t *attr, 
                      zoidfs::zoidfs_op_hint_t * op_hint) {ASSERT ("SetAttr Not Implemented" != 0); }

         void cblookup(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *parent_handle,
                     const char *component_name, 
                     const char *full_path,
                     zoidfs::zoidfs_handle_t *handle,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
         void cbreadlink(const IOFWDClientCB & cb,
                       int * ret,
                       const zoidfs::zoidfs_handle_t *handle,  
                       char *buffer,
                       size_t buffer_length,
                       zoidfs::zoidfs_op_hint_t * op_hint) {ASSERT ("ReadLink Not Implemented" != 0);}
                       
         void cbcommit(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *handle,
                     zoidfs::zoidfs_op_hint_t * op_hint) {ASSERT ("Commit Not Implemented" != 0); }
                     
         void cbcreate(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *parent_handle,
                     const char *component_name, 
                     const char *full_path,
                     const zoidfs::zoidfs_sattr_t *sattr, 
                     zoidfs::zoidfs_handle_t *handle,
                     int *created,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                    
         void cbremove(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *parent_handle,
                     const char *component_name, const char *full_path,
                     zoidfs::zoidfs_cache_hint_t *parent_hint,
                     zoidfs::zoidfs_op_hint_t * op_hint) {ASSERT ("Remove Not Implemented" != 0); }
                     
         void cbrename(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *from_parent_handle,
                     const char *from_component_name,
                     const char *from_full_path,
                     const zoidfs::zoidfs_handle_t *to_parent_handle,
                     const char *to_component_name,
                     const char *to_full_path,
                     zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                     zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                     zoidfs::zoidfs_op_hint_t * op_hint) {ASSERT ("Rename Not Implemented" != 0); }
                     
         void cblink(const IOFWDClientCB & cb,
                   int * ret,
                   const zoidfs::zoidfs_handle_t *from_parent_handle,
                   const char *from_component_name,
                   const char *from_full_path,
                   const zoidfs::zoidfs_handle_t *to_parent_handle,
                   const char *to_component_name,
                   const char *to_full_path,
                   zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                   zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                   zoidfs::zoidfs_op_hint_t * op_hint) {ASSERT ("Link Not Implemented" != 0); }
                   
         void cbsymlink(const IOFWDClientCB & cb,
                      int * ret,
                      const zoidfs::zoidfs_handle_t *from_parent_handle,
                      const char *from_component_name,
                      const char *from_full_path,
                      const zoidfs::zoidfs_handle_t *to_parent_handle,
                      const char *to_component_name,
                      const char *to_full_path,
                      const zoidfs::zoidfs_sattr_t *sattr,
                      zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                      zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                      zoidfs::zoidfs_op_hint_t * op_hint) {ASSERT ("symlink Not Implemented" != 0); }
                      
         void cbmkdir(const IOFWDClientCB & cb,
                    int * ret,
                    const zoidfs::zoidfs_handle_t *parent_handle,
                    const char *component_name, const char *full_path,
                    const zoidfs::zoidfs_sattr_t *sattr,
                    zoidfs::zoidfs_cache_hint_t *parent_hint,
                    zoidfs::zoidfs_op_hint_t * op_hint) {ASSERT ("mkdir Not Implemented" != 0); }
                            
         void cbreaddir(const IOFWDClientCB & cb,
                      int * ret,
                      const zoidfs::zoidfs_handle_t *parent_handle,
                      zoidfs::zoidfs_dirent_cookie_t cookie, size_t *entry_count_,
                      zoidfs::zoidfs_dirent_t *entries, uint32_t flags,
                      zoidfs::zoidfs_cache_hint_t *parent_hint,
                      zoidfs::zoidfs_op_hint_t * op_hint) {ASSERT ("readdir Not Implemented" != 0); }

         void cbresize(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *handle, 
                     zoidfs::zoidfs_file_size_t size,
                     zoidfs::zoidfs_op_hint_t * op_hint) {ASSERT ("Resize Not Implemented" != 0); }
                     
                   
         void cbread(const IOFWDClientCB & cb,
                   int * ret,
                   const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                   void *mem_starts[], const size_t mem_sizes[],
                   size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                   zoidfs::zoidfs_file_size_t file_sizes[],
                   zoidfs::zoidfs_op_hint_t * op_hint);
                    
         void cbwrite(const IOFWDClientCB & cb,
                    int * ret,
                    const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                    const void *mem_starts[], const size_t mem_sizes[],
                    size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                    zoidfs::zoidfs_file_ofs_t file_sizes[],
                    zoidfs::zoidfs_op_hint_t * op_hint);


         void cbinit(const IOFWDClientCB & cb, int * ret, 
                    zoidfs::zoidfs_op_hint_t * op_hint) {ASSERT ("init Not Implemented" != 0); }


         void cbfinalize(const IOFWDClientCB & cb, int * ret, 
                    zoidfs::zoidfs_op_hint_t * op_hint) {ASSERT ("Finalize Not Implemented" != 0); }

         void cbnull(const IOFWDClientCB & cb, int * ret, 
                    zoidfs::zoidfs_op_hint_t * op_hint) {ASSERT ("null Not Implemented" != 0); }
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
                    sm_ = sm;
                }

                void call(zoidfs::zoidfs_comp_mask_t mask, const
                        iofwdevent::CBException & cbexception)
                {
                    fprintf(stderr,"CBSM Callback\n");
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
                    sm_(sm),
                    wcb_(boost::bind(&CBClient::cbWrapper, this, _1, _2))
                {
                   cb_ = cb;
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

                IOFWDClientCB cb_;
                sm::SMClient * sm_;
                const IOFWDClientCB wcb_;
         };
        
        static void cbWrapper(CBSMWrapper * cbsm,
            zoidfs::zoidfs_comp_mask_t mask,
            const iofwdevent::CBException & cbexception);

      protected:
         iofwdutil::IOFWDLogSource & log_;
         CommStream & net_;
         net::AddressPtr addr_;
         bool poll_;
         boost::shared_ptr<iofwd::RPCClient> client_;
         boost::shared_ptr<sm::SMManager> smm_;
   };

   //========================================================================
}

#endif
