#ifndef IOFWDUTIL_IOFSLKEY_HH
#define IOFWDUTIL_IOFSLKEY_HH

#include <string>
#include "zoidfs/util/zoidfs-c-util.h"

namespace iofwdutil
{
    class IOFSLKey
    {
        public:
            IOFSLKey(zoidfs::zoidfs_handle_t * ffh, zoidfs::zoidfs_handle_t
                * fph = NULL, std::string fcn = std::string(""), std::string fp =
                std::string(""), std::string dk = std::string("")) :
                    f_handle_set_(false),
                    file_p_handle_set_(false),
                    file_c_name_(std::string(fcn)),
                    file_path_(fp),
                    data_key_(dk)
            {
                if(ffh)
                {
                    setFileHandle(ffh);
                }
                else
                {
                    clearHandle(&f_handle_, f_handle_set_);
                }

                if(fph)
                {
                    setParentHandle(fph);
                }
                else
                {
                    clearHandle(&file_p_handle_, file_p_handle_set_);
                }
            }

            IOFSLKey() :
                f_handle_set_(false),
                file_p_handle_set_(false),
                file_c_name_(std::string("")), file_path_(std::string("")),
                data_key_(std::string(""))
            {
                clearHandle(&f_handle_, f_handle_set_);
                clearHandle(&file_p_handle_, file_p_handle_set_);
            }

            IOFSLKey(const IOFSLKey & r) : f_handle_set_(false),
                file_p_handle_set_(false)
            {
                if(r.f_handle_set_)
                {
                    setFileHandle(&(r.f_handle_));
                }
                if(r.file_p_handle_set_)
                {
                    setParentHandle(&(r.file_p_handle_));
                }
                file_c_name_ = r.file_c_name_;
                file_path_ = r.file_path_;
                data_key_ = r.data_key_;
            }

            ~IOFSLKey();

            void setParentHandle(const zoidfs::zoidfs_handle_t * fph)
            {
                if(!fph)
                    return;

                file_p_handle_ = *fph;
                file_p_handle_set_ = true;
            }

            void setComponentName(const std::string fcn)
            {
                file_c_name_ = fcn;
            }

            void setFileHandle(const zoidfs::zoidfs_handle_t * fh)
            {
                if(!fh)
                    return;

                f_handle_ = *fh;
                f_handle_set_ = true;
            }

            void setFilePath(const std::string fp)
            {
                file_path_ = fp;
            }

            void setDataKey(const std::string dk)
            {
                data_key_ = dk;
            }

            zoidfs::zoidfs_handle_t getParentHandle() const
            {
                return file_p_handle_; 
            }

            std::string getComponentName() const
            {
                return file_c_name_;
            }

            zoidfs::zoidfs_handle_t getFileHandle() const
            {
                return f_handle_; 
            }

            std::string getFilePath() const
            {
                return file_path_;
            }

            std::string getDataKey() const
            {
                return data_key_;
            }

            bool operator== (const IOFSLKey & r) const
            {
                bool ret = false;
            
                if(data_key_ == r.data_key_)
                {
                    ret = true;
                }
                else
                {
                    ret = false;
                }

                /* check the full file handle */
                if(f_handle_set_ && r.f_handle_set_)
                {
                    if(memcmp(&f_handle_, &r.f_handle_,
                        sizeof(zoidfs::zoidfs_handle_t)) == 0)
                    {
                        return ret && true;
                    }
                    return ret && false;
                }
                /* check the parent / component info */
                else if(file_p_handle_set_ && r.file_p_handle_set_)
                {
                    if(memcmp(&file_p_handle_, &r.file_p_handle_,
                        sizeof(zoidfs::zoidfs_handle_t)) == 0)
                    {
                        if(file_c_name_ == r.file_c_name_)
                        {
                            return ret && true;
                        }
                    }
                    return ret && false;
                }
                /* check the file path */
                else if(file_path_ != str_sentinel_ && r.file_path_ !=
                    str_sentinel_)
                {
                    if(file_path_ == r.file_path_)
                    {
                        return ret && true;
                    }
                    return ret && false;
                }

                return ret && false;
            }

            bool operator< (const IOFSLKey & r) const
            {
                bool ret = false;
            
                if(data_key_ == r.data_key_)
                {
                    ret = true;
                }
                else
                {
                    ret = false;
                }

                /* check the full file handle */
                if(f_handle_set_ && r.f_handle_set_)
                {
                    if(memcmp(&f_handle_, &r.f_handle_,
                        sizeof(zoidfs::zoidfs_handle_t)) < 0)
                    {
                        return ret && true;
                    }
                    return ret && false;
                }
                /* check the parent / component info */
                else if(file_p_handle_set_ && r.file_p_handle_set_)
                {
                    if(memcmp(&file_p_handle_, &r.file_p_handle_,
                        sizeof(zoidfs::zoidfs_handle_t)) < 0)
                    {
                        if(file_c_name_ < r.file_c_name_)
                        {
                            return ret && true;
                        }
                    }
                    return ret && false;
                }
                /* check the file path */
                else if(file_path_ != str_sentinel_ && r.file_path_ !=
                    str_sentinel_)
                {
                    if(file_path_ < r.file_path_)
                    {
                        return ret && true;
                    }
                    return ret && false;
                }

                return ret && false;
            }

        protected:
            void clearHandle(zoidfs::zoidfs_handle_t * h, bool & s)
            {
                memset(h, 0, sizeof(zoidfs::zoidfs_handle_t));
                s = false;
            }

            /* file id info */
            zoidfs::zoidfs_handle_t f_handle_; /* the file handle */
            bool f_handle_set_;

            /* parent / component handle */
            zoidfs::zoidfs_handle_t file_p_handle_;
            bool file_p_handle_set_;
            std::string file_c_name_;

            /* the file path */ 
            std::string file_path_;
   
            /* data key for this file */ 
            std::string data_key_;

            static const std::string str_sentinel_;
    };
}

#endif
