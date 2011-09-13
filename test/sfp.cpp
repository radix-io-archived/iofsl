#include "zoidfs/zoidfs.h"
#include <iostream>

#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>


#include "iofwdutil/always_assert.hh"

#include "zoidfs/hints/zoidfs-hints.h"

using namespace zoidfs;
using namespace zoidfs::hints;

using namespace std;

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

   void setHint (zoidfs::zoidfs_op_hint_t * hint, const std::string & name,
         uint64_t val)
   {
      setHint (hint, name, boost::lexical_cast<std::string>(val));
   }
}


bool createSFP (const zoidfs_handle_t * handle, uint64_t sfp_id)
{
   zoidfs_op_hint_t hint;
   zoidfs_hint_create (&hint);
   setHint (&hint, ZOIDFS_SFP_OP, "CREATE");
   setHint (&hint, ZOIDFS_SFP_SFPID, sfp_id);
   setHint (&hint, ZOIDFS_SFP_VAL, 0);

   zoidfs_sattr_t sattr;
   zoidfs_attr_t attr;
   int ret = zoidfs_setattr (handle, &sattr, &attr, &hint);

   zoidfs_hint_free (&hint);
   return ret;
}

bool removeSFP (const zoidfs_handle_t * handle, uint64_t sfp_id)
{
   zoidfs_op_hint_t hint;
   zoidfs_hint_create (&hint);
   setHint (&hint, ZOIDFS_SFP_OP, "REMOVE");
   setHint (&hint, ZOIDFS_SFP_SFPID, sfp_id);
   setHint (&hint, ZOIDFS_SFP_VAL, 0);

   zoidfs_sattr_t sattr;
   zoidfs_attr_t attr;
   int ret = zoidfs_setattr (handle, &sattr, &attr, &hint);

   zoidfs_hint_free (&hint);
   return ret;
}

zoidfs_file_ofs_t stepSFP (const zoidfs_handle_t * handle, uint64_t sfp_id,
      zoidfs_file_ofs_t step)
{
   zoidfs_op_hint_t hint;
   zoidfs_hint_create (&hint);
   setHint (&hint, ZOIDFS_SFP_OP, "FETCH_AND_ADD");
   setHint (&hint, ZOIDFS_SFP_SFPID, sfp_id);
   setHint (&hint, ZOIDFS_SFP_VAL, step);

   zoidfs_sattr_t sattr;
   zoidfs_attr_t attr;
   zoidfs_setattr (handle, &sattr, &attr, &hint);

   boost::optional<std::string> response = getHint (&hint, ZOIDFS_SFP_VAL);
   ALWAYS_ASSERT(response);

   zoidfs_hint_free (&hint);
   return boost::lexical_cast<zoidfs::zoidfs_file_ofs_t> (*response);
}


int main (int argc, char ** args)
{
   if (argc != 3)
   {
      cerr << "Need filename and sfpid!\n";
      exit (1);
   }

   const std::string filename = args[1];
   const uint64_t sfpid = boost::lexical_cast<uint64_t> (args[2]);

   zoidfs_init ();

   zoidfs_handle_t handle;
   zoidfs_lookup (0, 0, filename.c_str(), &handle, 0);

  
   createSFP (&handle, sfpid);

   for (size_t i=0; i<10; ++i)
   {
      cout << "After step: " << stepSFP (&handle, sfpid, i+1) << endl;
   }

   removeSFP (&handle, sfpid);

   zoidfs_finalize ();
   return 0;
}
