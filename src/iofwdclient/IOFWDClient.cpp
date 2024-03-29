#include "iofwdclient/IOFWDClient.hh"

#include "iofwdclient/PollQueue.hh"
#include "iofwdclient/ASClient.hh"
#include "iofwdclient/SyncClient.hh"

#include "iofwd/RPCClient.hh"

#include <boost/shared_ptr.hpp>

#include "iofwdutil/IOFWDLog.hh"
#include <stdio.h>

using namespace zoidfs;

namespace iofwdclient
{
   //========================================================================
   IOFWDClient::IOFWDClient ()
      : log_ (iofwdutil::IOFWDLog::getSource ())
   {

   }
   IOFWDClient::IOFWDClient (CommStream & net, net::AddressPtr addr, bool poll)
      : log_ (iofwdutil::IOFWDLog::getSource ())
   {
      ZLOG_DEBUG (log_, "Initializing IOFWDClient");
//      cbclient_ = new CBClient(net, poll);
      asclient_.reset(new ASClient (log_, *(new CBClient(log_, net, addr, poll))) );
      sclient_.reset( new SyncClient (log_, *asclient_));
   }

   IOFWDClient::~IOFWDClient ()
   {
      ZLOG_DEBUG (log_, "Shutting down IOFWDClient");
   }

   // @TODO: This needs to be changed, possibly use the CommStream class?
//   void IOFWDClient::RPCMode (boost::shared_ptr<iofwd::RPCClient> rpcclient,
//                              net::AddressPtr addr)
//   {
//      asclient_->setRPCMode (rpcclient, addr);
//      sclient_->setRPCMode (rpcclient, addr);
//   }

   // -----------------------------------------------------------------
   // ------------- blocking ZoidFS functions -------------------------
   // -----------------------------------------------------------------

   int IOFWDClient::getattr (const zoidfs_handle_t * handle,
         zoidfs_attr_t * attr, zoidfs_op_hint_t * op_hint)
   {
      return sclient_->getattr (handle, attr, op_hint);
   }

   int IOFWDClient::setattr(const zoidfs::zoidfs_handle_t *handle,
                            const zoidfs::zoidfs_sattr_t *sattr,
                            zoidfs::zoidfs_attr_t *attr, 
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->setattr(handle,sattr, attr, op_hint);
   }

   int IOFWDClient::lookup(const zoidfs::zoidfs_handle_t *parent_handle,
                           const char *component_name, 
                           const char *full_path,
                           zoidfs::zoidfs_handle_t *handle,  
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->lookup(parent_handle, component_name, full_path, 
                              handle, op_hint);
   }

   int IOFWDClient::readlink(const zoidfs::zoidfs_handle_t *handle, 
                             char *buffer,
                             size_t buffer_length,
                             zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->readlink(handle, buffer, buffer_length, op_hint);
   }

                      
   int IOFWDClient::commit(const zoidfs::zoidfs_handle_t *handle,
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->commit (handle, op_hint);
   }
   
   int IOFWDClient::create(const zoidfs::zoidfs_handle_t *parent_handle,
                           const char *component_name, 
                           const char *full_path,
                           const zoidfs::zoidfs_sattr_t *sattr, 
                           zoidfs::zoidfs_handle_t *handle,
                           int *created,
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->create (parent_handle, component_name, full_path, 
                               sattr, handle, created, op_hint);
   }  
   
              
   int IOFWDClient::remove(const zoidfs::zoidfs_handle_t *parent_handle,
                           const char *component_name, const char *full_path,
                           zoidfs::zoidfs_cache_hint_t *parent_hint,
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->remove(parent_handle, component_name, full_path, 
                              parent_hint, op_hint);   
   }

   int IOFWDClient::rename(const zoidfs::zoidfs_handle_t *from_parent_handle,
                           const char *from_component_name,
                           const char *from_full_path,
                           const zoidfs::zoidfs_handle_t *to_parent_handle,
                           const char *to_component_name,
                           const char *to_full_path,
                           zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                           zoidfs::zoidfs_cache_hint_t *to_parent_hint,     
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->rename(from_parent_handle, from_component_name, 
                              from_full_path, to_parent_handle,
                              to_component_name, to_full_path,
                              from_parent_hint, to_parent_hint, op_hint);
   }

   int IOFWDClient::link(const zoidfs::zoidfs_handle_t *from_parent_handle,
                         const char *from_component_name,
                         const char *from_full_path,
                         const zoidfs::zoidfs_handle_t *to_parent_handle,
                         const char *to_component_name,
                         const char *to_full_path,
                         zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                         zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                         zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->link(from_parent_handle, from_component_name,
                            from_full_path, to_parent_handle, 
                            to_component_name, to_full_path, 
                            from_parent_hint, to_parent_hint,
                            op_hint);
   }
        
   int IOFWDClient::symlink(const zoidfs::zoidfs_handle_t *from_parent_handle,
                            const char *from_component_name,
                            const char *from_full_path,
                            const zoidfs::zoidfs_handle_t *to_parent_handle,
                            const char *to_component_name,
                            const char *to_full_path,
                            const zoidfs::zoidfs_sattr_t *sattr,
                            zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                            zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->symlink ( from_parent_handle, from_component_name,
                                 from_full_path, to_parent_handle, 
                                 to_component_name, to_full_path,
                                 sattr, from_parent_hint, to_parent_hint,
                                 op_hint);
   }
   
                      
   int IOFWDClient::mkdir(const zoidfs::zoidfs_handle_t *parent_handle,
                          const char *component_name, const char *full_path,
                          const zoidfs::zoidfs_sattr_t *sattr,
                          zoidfs::zoidfs_cache_hint_t *parent_hint,
                          zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->mkdir(parent_handle, component_name, full_path,
                             sattr, parent_hint, op_hint);
   }


   int IOFWDClient::readdir(const zoidfs::zoidfs_handle_t *parent_handle,
                            zoidfs::zoidfs_dirent_cookie_t cookie, 
                            size_t *entry_count_,
                            zoidfs::zoidfs_dirent_t *entries, uint32_t flags,
                            zoidfs::zoidfs_cache_hint_t *parent_hint,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->readdir(parent_handle, cookie, entry_count_, entries, 
                               flags, parent_hint, op_hint);
   }

   int IOFWDClient::resize(const zoidfs::zoidfs_handle_t *handle, 
                           zoidfs::zoidfs_file_size_t size,
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->resize(handle, size, op_hint);
   }


   int IOFWDClient::read(const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                         void *mem_starts[], const size_t mem_sizes[],
                         size_t file_count, 
                         const zoidfs::zoidfs_file_ofs_t file_starts[],
                         zoidfs::zoidfs_file_size_t file_sizes[],
                         zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->read( handle, mem_count, mem_starts, mem_sizes, file_count,
                             file_starts, file_sizes, op_hint);
   }

   int IOFWDClient::write(const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                          const void *mem_starts[], const size_t mem_sizes[],
                          size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                          zoidfs::zoidfs_file_ofs_t file_sizes[],
                          zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->write(handle, mem_count, mem_starts, mem_sizes, 
                             file_count, file_starts, file_sizes, op_hint);
   }

   int IOFWDClient::null ( zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->null (op_hint);
   }

   int IOFWDClient::finalize (zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->finalize (op_hint);
   }
   
   int IOFWDClient::init (zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return sclient_->init (op_hint);
   }
   

   // -----------------------------------------------------------------
   // -------------- zoidfs async methods -----------------------------
   // -----------------------------------------------------------------


   int IOFWDClient::igetattr(zoidfs::zoidfs_request_t * request,
                   const zoidfs::zoidfs_handle_t * handle,
                   zoidfs::zoidfs_attr_t * attr,
                   zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->igetattr (request, handle, attr, op_hint);
   }


   int IOFWDClient::isetattr(zoidfs::zoidfs_request_t * request,
                             const zoidfs::zoidfs_handle_t *handle,
                             const zoidfs::zoidfs_sattr_t *sattr,
                             zoidfs::zoidfs_attr_t *attr, 
                             zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->isetattr(request,handle,sattr, attr, op_hint);
   }


   int IOFWDClient::ilookup(zoidfs::zoidfs_request_t * request,
                            const zoidfs::zoidfs_handle_t *parent_handle,
                            const char *component_name, 
                            const char *full_path,
                            zoidfs::zoidfs_handle_t *handle,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->ilookup(request, parent_handle, component_name, 
                              full_path, handle, op_hint);
   }
               
   int IOFWDClient::ireadlink(zoidfs::zoidfs_request_t * request,
                              const zoidfs::zoidfs_handle_t *handle,  
                              char *buffer,   
                              size_t buffer_length,
                              zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->ireadlink(request, handle, buffer, buffer_length, 
                                 op_hint);
   }
                 
   int IOFWDClient::icommit(zoidfs::zoidfs_request_t * request,
                            const zoidfs::zoidfs_handle_t *handle,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->icommit( request, handle, op_hint);
   }
               
   int IOFWDClient::icreate(zoidfs::zoidfs_request_t * request,
                            const zoidfs::zoidfs_handle_t *parent_handle,
                            const char *component_name, 
                            const char *full_path,
                            const zoidfs::zoidfs_sattr_t *sattr, 
                            zoidfs::zoidfs_handle_t *handle,
                            int *created,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->icreate (request, parent_handle, component_name, 
                                 full_path, sattr, handle, created, op_hint);
   }
              
   int IOFWDClient::iremove(zoidfs::zoidfs_request_t * request,
                            const zoidfs::zoidfs_handle_t *parent_handle,
                            const char *component_name, const char *full_path,
                            zoidfs::zoidfs_cache_hint_t *parent_hint,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->iremove (request, parent_handle, component_name, 
                                 full_path, parent_hint, op_hint);
   }
               
   int IOFWDClient::irename(zoidfs::zoidfs_request_t * request,
                            const zoidfs::zoidfs_handle_t *from_parent_handle,
                            const char *from_component_name,
                            const char *from_full_path,
                            const zoidfs::zoidfs_handle_t *to_parent_handle,
                            const char *to_component_name,
                            const char *to_full_path,
                            zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                            zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->irename ( request, from_parent_handle, 
                                  from_component_name, from_full_path, 
                                  to_parent_handle, to_component_name, 
                                  to_full_path, from_parent_hint, 
                                  to_parent_hint, op_hint);
   }
               
   int IOFWDClient::ilink(zoidfs::zoidfs_request_t * request,
                          const zoidfs::zoidfs_handle_t *from_parent_handle,
                          const char *from_component_name,
                          const char *from_full_path,
                          const zoidfs::zoidfs_handle_t *to_parent_handle,
                          const char *to_component_name,
                          const char *to_full_path,
                          zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                          zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                          zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->ilink(request, from_parent_handle, from_component_name,
                              from_full_path, to_parent_handle, 
                              to_component_name, to_full_path, 
                              from_parent_hint, to_parent_hint,
                              op_hint);   
   }
             
   int IOFWDClient::isymlink(zoidfs::zoidfs_request_t * request,
                             const zoidfs::zoidfs_handle_t *from_parent_handle,
                             const char *from_component_name,
                             const char *from_full_path,
                             const zoidfs::zoidfs_handle_t *to_parent_handle,
                             const char *to_component_name,
                             const char *to_full_path,
                             const zoidfs::zoidfs_sattr_t *sattr,
                             zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                             zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                             zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->isymlink ( request, from_parent_handle, from_component_name,
                                  from_full_path, to_parent_handle, 
                                  to_component_name, to_full_path,
                                  sattr, from_parent_hint, to_parent_hint,
                                  op_hint);

   }
   int IOFWDClient::imkdir(zoidfs::zoidfs_request_t * request,
                           const zoidfs::zoidfs_handle_t *parent_handle,
                           const char *component_name, const char *full_path,
                           const zoidfs::zoidfs_sattr_t *sattr,
                           zoidfs::zoidfs_cache_hint_t *parent_hint,
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->imkdir(request, parent_handle, component_name, full_path,
                              sattr, parent_hint, op_hint);
   }
                      
   int IOFWDClient::ireaddir(zoidfs::zoidfs_request_t * request,
                             const zoidfs::zoidfs_handle_t *parent_handle,
                             zoidfs::zoidfs_dirent_cookie_t cookie, size_t *entry_count_,
                             zoidfs::zoidfs_dirent_t *entries, uint32_t flags,
                             zoidfs::zoidfs_cache_hint_t *parent_hint,
                             zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->ireaddir(request, parent_handle, cookie, entry_count_, 
                                 entries, flags, parent_hint, op_hint);
   }

   int IOFWDClient::iresize(zoidfs::zoidfs_request_t * request,
                            const zoidfs::zoidfs_handle_t *handle, 
                            zoidfs::zoidfs_file_size_t size,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->iresize(request, handle, size, op_hint);
   }
               
             
   int IOFWDClient::iread(zoidfs::zoidfs_request_t * request,
                          const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                          void *mem_starts[], const size_t mem_sizes[],
                          size_t file_count, 
                          const zoidfs::zoidfs_file_ofs_t file_starts[],
                          zoidfs::zoidfs_file_size_t file_sizes[],
                          zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->iread( request, handle, mem_count, mem_starts,
                             mem_sizes, file_count, file_starts, file_sizes, 
                             op_hint);
   }
              
   int IOFWDClient::iwrite(zoidfs::zoidfs_request_t * request,
                         const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                         const void *mem_starts[], const size_t mem_sizes[],
                         size_t file_count, 
                         const zoidfs::zoidfs_file_ofs_t file_starts[],
                         zoidfs::zoidfs_file_ofs_t file_sizes[],
                         zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->iwrite(request, handle, mem_count, mem_starts, 
                               mem_sizes, file_count, file_starts, file_sizes, 
                               op_hint);
   }
            
   int IOFWDClient::inull ( zoidfs::zoidfs_request_t * request,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->inull (request, op_hint);
   }

   int IOFWDClient::ifinalize (zoidfs::zoidfs_request_t * request,
                               zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->ifinalize (request, op_hint);
   }
   
   int IOFWDClient::iinit (zoidfs::zoidfs_request_t * request,
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->iinit (request, op_hint);
   }


   int IOFWDClient::request_test (zoidfs::zoidfs_request_t request,
                   zoidfs::zoidfs_timeout_t timeout,
                   zoidfs::zoidfs_comp_mask_t mask)
   {
      return asclient_->request_test (request, timeout, mask);
   }

   int IOFWDClient::request_get_error (zoidfs::zoidfs_request_t request,
         int * error)
   {
      return asclient_->request_get_error (request, error);
   }

   int IOFWDClient::request_get_comp_state (zoidfs::zoidfs_request_t r,
               zoidfs::zoidfs_comp_mask_t * m)
   {
      return asclient_->request_get_comp_state (r, m);
   }

   int IOFWDClient::request_free (zoidfs::zoidfs_request_t * request)
   {
      return asclient_->request_free (request);
   }

   /* IOFWDClient wrappers */

   extern "C" void * IOFWDClient_cwrapper_allocate(char * address,
           char * configfile)
   {
       iofwdevent::SingleCompletion block;
       net::Net * net = NULL;
       net::AddressPtr addr;
       registerIofwdFactoryClients();
       iofwd::service::ServiceManager & man =
           iofwd::service::ServiceManager::instance();

       man.setParam("config.configfile", std::string(configfile));

       boost::shared_ptr<iofwd::Net>
           netservice(man.loadService<iofwd::Net>("net"));
       net = netservice->getNet();
       net->lookup(address, &addr, block);
       block.wait();

       return new iofwdclient::IOFWDClient(*(new iofwdclient::CommStream()),
               addr, true);
   }

   extern "C" void IOFWDClient_cwrapper_free(IOFWDClient * c)
   {
       delete c;
   }

   extern "C" int IOFWDClient_cwrapper_write(IOFWDClient * c,
           const zoidfs::zoidfs_handle_t * handle,
           size_t mem_count,
           const void *mem_starts[],
           const size_t mem_sizes[],
           size_t file_count, 
           const zoidfs::zoidfs_file_ofs_t file_starts[],
           zoidfs::zoidfs_file_ofs_t file_sizes[],
           zoidfs::zoidfs_op_hint_t * op_hint)
   {
       return c->write(handle, mem_count,
               mem_starts, mem_sizes, file_count, file_starts, file_sizes,
               op_hint);
   }

   extern "C" int IOFWDClient_cwrapper_init(IOFWDClient * c,
           zoidfs::zoidfs_op_hint_t * op_hint)
   {
       return c->init(op_hint);
   }
   
   extern "C" int IOFWDClient_cwrapper_finalize(IOFWDClient * c,
           zoidfs::zoidfs_op_hint_t * op_hint)
   {
       return c->finalize(op_hint);
   }

   extern "C" int IOFWDClient_cwrapper_create(IOFWDClient * c,
                    const zoidfs::zoidfs_handle_t * parent_handle,
                    const char * component_name,
                    const char * full_path,
                    const zoidfs::zoidfs_sattr_t * sattr,
                    zoidfs::zoidfs_handle_t * handle,
                    int * created,
                    zoidfs::zoidfs_op_hint_t * op_hint)
   {
       return c->create(parent_handle,
               component_name, full_path, sattr, handle, created, op_hint);
   }

   extern "C" int IOFWDClient_cwrapper_lookup(IOFWDClient * c,
                    const zoidfs::zoidfs_handle_t * parent_handle,
                    const char * component_name,
                    const char * full_path,
                    zoidfs::zoidfs_handle_t * handle,
                    zoidfs::zoidfs_op_hint_t * op_hint)
   {
       return c->lookup(parent_handle,
               component_name, full_path, handle, op_hint);
   }

   extern "C" int IOFWDClient_cwrapper_remove(IOFWDClient * c,
           const zoidfs::zoidfs_handle_t *parent_handle,
           const char *component_name, const char *full_path,
           zoidfs::zoidfs_cache_hint_t *parent_hint,
           zoidfs::zoidfs_op_hint_t * op_hint)
   {
       return c->remove(parent_handle,
               component_name, full_path, parent_hint, op_hint);
   }

   extern "C" int IOFWDClient_cwrapper_setattr(IOFWDClient * c, const zoidfs::zoidfs_handle_t * handle,
        const zoidfs::zoidfs_sattr_t * sattr, zoidfs::zoidfs_attr_t * attr,
        zoidfs::zoidfs_op_hint_t * op_hint)
   {
        return c->setattr(handle, sattr, attr, op_hint); 
   }

   extern "C" int IOFWDClient_cwrapper_getattr(IOFWDClient * c, const
        zoidfs::zoidfs_handle_t * handle, zoidfs::zoidfs_attr_t * attr,
        zoidfs::zoidfs_op_hint_t * op_hint)
   {
        return c->getattr(handle, attr, op_hint); 
   }

   extern "C" int IOFWDClient_cwrapper_null(IOFWDClient * c,
           zoidfs::zoidfs_op_hint_t * op_hint)
   {
        return c->null(op_hint); 
   }
   
   extern "C" int IOFWDClient_cwrapper_read(IOFWDClient * c,
                const zoidfs::zoidfs_handle_t * handle,
                size_t mem_count,
                void * mem_starts[],
                const size_t mem_sizes[],
                size_t file_count,
                const zoidfs::zoidfs_file_ofs_t file_starts[],
                zoidfs::zoidfs_file_size_t file_sizes[],
                zoidfs::zoidfs_op_hint_t * op_hint )
    {
        return c->read(handle, mem_count, mem_starts, mem_sizes, file_count,
            file_starts, file_sizes, op_hint);
    }

    extern "C" int IOFWDClient_cwrapper_commit(IOFWDClient * c,
                  const zoidfs::zoidfs_handle_t * handle,
                  zoidfs::zoidfs_op_hint_t * op_hint )
    {
        return c->commit(handle, op_hint);
    }

    extern "C" int IOFWDClient_cwrapper_rename(IOFWDClient * c,
                  const zoidfs::zoidfs_handle_t * from_parent_handle,
                  const char * from_component_name,
                  const char * from_full_path,
                  const zoidfs::zoidfs_handle_t * to_parent_handle,
                  const char * to_component_name,
                  const char * to_full_path,
                  zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                  zoidfs::zoidfs_cache_hint_t * to_parent_hint,
                  zoidfs::zoidfs_op_hint_t * op_hint )
    {
        return c->rename(from_parent_handle, from_component_name,
            from_full_path, to_parent_handle, to_component_name, to_full_path,
            from_parent_hint, to_parent_hint, op_hint);
    }

    extern "C" int IOFWDClient_cwrapper_link(IOFWDClient * c,
                const zoidfs::zoidfs_handle_t * from_parent_handle,
                const char * from_component_name,
                const char * from_full_path,
                const zoidfs::zoidfs_handle_t * to_parent_handle,
                const char * to_component_name,
                const char * to_full_path,
                zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                zoidfs::zoidfs_cache_hint_t * to_parent_hint,
                zoidfs::zoidfs_op_hint_t * op_hint )
    {
        return c->link(from_parent_handle, from_component_name, from_full_path,
            to_parent_handle, to_component_name, to_full_path, from_parent_hint,
            to_parent_hint, op_hint);
    }

    extern "C" int IOFWDClient_cwrapper_symlink(IOFWDClient * c,
                   const zoidfs::zoidfs_handle_t * from_parent_handle,
                   const char * from_component_name,
                   const char * from_full_path,
                   const zoidfs::zoidfs_handle_t * to_parent_handle,
                   const char * to_component_name,
                   const char * to_full_path,
                   const zoidfs::zoidfs_sattr_t * attr,
                   zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                   zoidfs::zoidfs_cache_hint_t * to_parent_hint,
                   zoidfs::zoidfs_op_hint_t * op_hint )
    {
        return c->symlink(from_parent_handle, from_component_name,
            from_full_path, to_parent_handle, to_component_name, to_full_path, attr,
            from_parent_hint, to_parent_hint, op_hint);
    }

    extern "C" int IOFWDClient_cwrapper_mkdir(IOFWDClient * c,
                 const zoidfs::zoidfs_handle_t * parent_handle,
                 const char * component_name,
                 const char * full_path,
                 const zoidfs::zoidfs_sattr_t * attr,
                 zoidfs::zoidfs_cache_hint_t * parent_hint,
                 zoidfs::zoidfs_op_hint_t * op_hint )
    {
        return c->mkdir(parent_handle, component_name, full_path, attr,
            parent_hint, op_hint);
    }

    extern "C" int IOFWDClient_cwrapper_readdir(IOFWDClient * c,
                   const zoidfs_handle_t * parent_handle,
                   zoidfs::zoidfs_dirent_cookie_t cookie,
                   size_t * entry_count,
                   zoidfs::zoidfs_dirent_t * entries,
                   uint32_t flags,
                   zoidfs::zoidfs_cache_hint_t * parent_hint,
                   zoidfs::zoidfs_op_hint_t * op_hint )
    {
        return c->readdir(parent_handle, cookie, entry_count, entries, flags,
            parent_hint, op_hint);
    }

    extern "C" int IOFWDClient_cwrapper_resize(IOFWDClient * c,
                  const zoidfs::zoidfs_handle_t * handle,
                  uint64_t size,
                  zoidfs::zoidfs_op_hint_t * op_hint )
    {
        return c->resize(handle, size, op_hint);
    }

   extern "C" int IOFWDClient_cwrapper_readlink(IOFWDClient * c,
           const zoidfs::zoidfs_handle_t * handle,
           char * buffer,
           size_t buffer_length,
           zoidfs::zoidfs_op_hint_t * op_hint)
   {
        return c->readlink(handle, buffer, buffer_length, op_hint);
   }

   //========================================================================
}
