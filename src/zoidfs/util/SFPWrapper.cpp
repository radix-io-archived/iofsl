#include <boost/format.hpp>

#include "iofwdutil/tools.hh"
#include "zoidfs-util.hh"
#include "iofwdutil/tools.hh"
#include "SFPWrapper.hh"
#include "iofwdutil/Factory.hh"
#include "iofwdutil/LinkHelper.hh"
#include "iofwd/service/ServiceManager.hh"
#include "iofwdutil/IOFWDLog.hh"

#include "zoidfs/hints/zoidfs-hints.h"
#include "iofwdevent/SingleCompletion.hh"

#include <boost/optional.hpp>

GENERIC_FACTORY_CLIENT(std::string,
      zoidfs::util::ZoidFSAPI,
      zoidfs::util::SFPWrapper,
      "sfp",
      sfp);


#define SFPOP_FETCH_AND_ADD  "FETCH_AND_ADD"
#define SFPOP_SET            "SET"
#define SFPOP_GET            "GET"
#define SFPOP_REMOVE         "REMOVE"
#define SFPOP_CREATE         "CREATE"

using namespace boost;
using namespace zoidfs;
using namespace zoidfs::hints;
using namespace iofwdevent;

using iofwd::extraservice::SFPService;


namespace zoidfs
{
   namespace util
   {
      //=======================================================================

      namespace
      {
         boost::optional<std::string> getHint (const zoidfs::zoidfs_op_hint_t *
               hint, const std::string & name)
         {
            int len;
            int flag;
            zoidfs_hint_get_valuelen (*hint, const_cast<char*>(name.c_str()),
                  &len, &flag);
            if (!flag)
               return boost::optional<std::string> ();

            char buf[len];
            zoidfs_hint_get (*hint, const_cast<char*>(name.c_str()),
                  len, buf, &flag);
            return std::string (buf);
         }

         void setHint (zoidfs::zoidfs_op_hint_t * hint, const std::string &
               name, const std::string & val)
         {
            zoidfs_hint_set (*hint, const_cast<char*>(name.c_str()),
                  const_cast<char*>(val.c_str()), val.size()+1);
         }

      }

      //=======================================================================


      SFPWrapper::SFPWrapper ()
         : log_ (iofwdutil::IOFWDLog::getSource ("sfp"))
      {
      }

      SFPWrapper::~SFPWrapper ()
      {
      }

      //=======================================================================


      void SFPWrapper::configure (const iofwdutil::ConfigFile & config)
      {
         std::string apiname = config.getKeyDefault ("blocking_api", "zoidfs");

         ZLOG_INFO(log_, format("Using zoidfs blocking API '%s'") %
            apiname);
         api_.reset (iofwdutil::Factory<
               std::string,
               zoidfs::util::ZoidFSAPI>::construct(apiname)());
         iofwdutil::Configurable::configure_if_needed (api_.get(),
               config.openSectionDefault(apiname.c_str()));

         sfp_service_ =
            iofwd::service::ServiceManager::instance().loadService<SFPService>("sfp");
      }

      int SFPWrapper::init()
      {
         int ret = api_->init();
         return ret;
      }

      int SFPWrapper::finalize(void)
      {
         int ret = api_->finalize ();
         return ret;
      }

      int SFPWrapper::null(void)
      {
         int ret =  api_->null ();
         return ret;
      }

      bool SFPWrapper::processSFPHint (const zoidfs_handle_t * handle,
            zoidfs_op_hint_t * op_hint, int * ret)
      {
         boost::optional<std::string> h = getHint (op_hint, ZOIDFS_SFP_OP);
         boost::optional<std::string> id = getHint (op_hint,
               ZOIDFS_SFP_SFPID);
         bool res;

         *ret = ZFS_OK;

         if (!h)
            return false;

         if (!id)
            return ZFSERR_OTHER;

         const uint64_t sfpid = boost::lexical_cast<uint64_t> (*id);
         SingleCompletion comp;

         if (*h == SFPOP_CREATE)
         {
            *ret = ZFS_OK;
            sfp_service_->createSFP (&res, handle, sfpid, 0, comp);
            comp.wait ();
            return true;
         }

         if (*h == SFPOP_REMOVE)
         {
            *ret = ZFS_OK;
            sfp_service_->removeSFP (&res, handle, sfpid, comp);
            comp.wait ();
            return true;
         }

         if (*h == SFPOP_GET)
         {
            zoidfs::zoidfs_file_ofs_t value;
            sfp_service_->updateSFP (&res, handle, sfpid, SFPService::SFP_FETCH,
                  &value, comp);
            comp.wait ();
            ZLOG_INFO (log_, str(boost::format("SFP_GET (%lu) returned %lu") %
                  sfpid % value));
            setHint (op_hint, ZOIDFS_SFP_VAL,
                  boost::lexical_cast<std::string>(value));
            *ret = ZFS_OK;
            return true;
         }

         // -- from this point we expect the ZOIDFS_SFP_VAL hint to be set as
         // well ---
         boost::optional<std::string> val = getHint (op_hint, ZOIDFS_SFP_VAL);

         if (!val)
         {
            *ret = ZFSERR_OTHER;
            return true;
         }

         zoidfs::zoidfs_file_ofs_t ofs =
            boost::lexical_cast<zoidfs::zoidfs_file_ofs_t> (*val);

         if (*h == SFPOP_SET)
         {
            ZLOG_INFO (log_, format("sfp_set (%lu): %lu") % sfpid % val);
            sfp_service_->updateSFP (&res, handle, sfpid, SFPService::SFP_SET,
                  &ofs, comp);
            comp.wait ();
            return true;
         }

         if (*h == SFPOP_FETCH_AND_ADD)
         {
            zoidfs_file_ofs_t old = ofs;
            sfp_service_->updateSFP (&res, handle, sfpid,
                  SFPService::SFP_FETCH_AND_ADD, &ofs, comp);
            comp.wait ();
            zoidfs::hints::zoidfs_hint_delete (*op_hint, ZOIDFS_SFP_VAL);
            setHint (op_hint, ZOIDFS_SFP_VAL,
                  boost::lexical_cast<std::string>(ofs));
            ZLOG_INFO (log_, format("sfp: fetch_and_add(%lu) +%lu fetch=%lu")
                  % sfpid % old % ofs);
            return true;
         }

         *ret = ZFSERR_OTHER;
         return true;
      }

      int SFPWrapper::setattr(const zoidfs_handle_t * handle,
            const zoidfs_sattr_t * a1,
            zoidfs_attr_t * a2,
            zoidfs_op_hint_t * op_hint)
      {
         int ret;

         // If we process a valid hint request, don't do the rest of the
         // setattr
         if (processSFPHint (handle, op_hint, &ret))
            return ret;

         return api_->setattr (handle, a1, a2, op_hint);
      }

      int SFPWrapper::getattr(const zoidfs_handle_t * handle,
            zoidfs_attr_t * attr,
            zoidfs_op_hint_t * op_hint)
      {
         int ret;
         if (processSFPHint (handle, op_hint, &ret))
            return ret;

         return api_->getattr (handle, attr, op_hint);
      }



      int SFPWrapper::lookup(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_handle_t * h,
            zoidfs_op_hint_t * op_hint)
      {
         int ret = api_->lookup (parent_handle, component_name, full_path, h, op_hint);
         return ret;
      }

      int SFPWrapper::readlink(const zoidfs_handle_t * handle,
            char * buffer,
            size_t buffer_length,
            zoidfs_op_hint_t * op_hint)
      {
         int ret = api_->readlink (handle, buffer, buffer_length, op_hint);
         return ret;
      }

      int SFPWrapper::read(const zoidfs_handle_t * handle,
            size_t mem_count,
            void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const zoidfs_file_ofs_t file_starts[],
            const zoidfs_file_size_t file_sizes[],
            zoidfs_op_hint_t * op_hint)
      {
        int ret = api_->read (handle, mem_count, mem_starts, mem_sizes,
               file_count, file_starts, file_sizes, op_hint);
         return ret;
      }

      int SFPWrapper::write(const zoidfs_handle_t * handle,
            size_t mem_count,
            const void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const zoidfs_file_ofs_t file_starts[],
            const zoidfs_file_size_t file_sizes[],
            zoidfs_op_hint_t * op_hint)
      {
         int ret = api_->write (handle, mem_count, mem_starts,
               mem_sizes, file_count, file_starts, file_sizes, op_hint);
         return ret;
      }

      int SFPWrapper::commit(const zoidfs_handle_t * handle,
            zoidfs_op_hint_t * op_hint)
      {
         int ret = api_->commit (handle, op_hint);
         return ret;
      }

      int SFPWrapper::create(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_handle_t * handle,
            int * created,
            zoidfs_op_hint_t * op_hint)
      {
         int ret= api_->create (parent_handle, component_name, full_path,
               attr, handle, created, op_hint);
         return ret;
      }

      int SFPWrapper::remove(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * op_hint)
      {
         int ret= api_->remove (parent_handle, component_name, full_path,
               parent_hint, op_hint);
         return ret;
      }

      int SFPWrapper::rename(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * op_hint)
      {
         int ret= api_->rename (from_parent_handle, from_component_name,
               from_full_path, to_parent_handle, to_component_name, to_full_path,
               from_parent_hint, to_parent_hint, op_hint);
         return ret;
      }

      int SFPWrapper::link(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * op_hint)
      {
         int ret = api_->link (from_parent_handle, from_component_name,
               from_full_path, to_parent_handle, to_component_name,
               to_full_path, from_parent_hint, to_parent_hint, op_hint);
         return ret;
      }


      int SFPWrapper::symlink(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * op_hint)
      {
         int ret = api_->symlink (from_parent_handle, from_component_name,
               from_full_path, to_parent_handle, to_component_name,
               to_full_path, attr, from_parent_hint, to_parent_hint, op_hint);
         return ret;
      }

      int SFPWrapper::mkdir(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * op_hint)
      {
         int ret= api_->mkdir (parent_handle, component_name, full_path,
               attr, parent_hint, op_hint);
         return ret;
      }

      int SFPWrapper::readdir(const zoidfs_handle_t * parent_handle,
            zoidfs_dirent_cookie_t cookie,
            size_t * entry_count,
            zoidfs_dirent_t * entries,
            uint32_t flags,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * op_hint)
      {
         int ret= api_->readdir (parent_handle, cookie, entry_count, entries, flags,
               parent_hint, op_hint);
         return ret;
      }

      int SFPWrapper::resize(const zoidfs_handle_t * handle,
            zoidfs_file_size_t size,
            zoidfs_op_hint_t * op_hint)
      {
         int ret=api_->resize (handle, size, op_hint);
         return ret;
      }

      //=======================================================================
   }

}
