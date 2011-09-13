#include "zoidfs/zoidfs.h"
#include "zoidfs/hints/zoidfs-hints.h"

#include "iofwdutil/always_assert.hh"

#include <iostream>

#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#include <vector>
#include <algorithm>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/density.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/foreach.hpp>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>


#include <mpi.h>
#include <ctime>



using namespace zoidfs;
using namespace zoidfs::hints;
using namespace boost;

using namespace std;
using namespace boost::accumulators;


const size_t opt_loops = 10;

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

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
   ALWAYS_ASSERT (ret == ZFS_OK && "createSFP returned error!");

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

zoidfs_file_ofs_t getSFP (const zoidfs_handle_t * handle, uint64_t sfp_id)
{
   zoidfs_op_hint_t hint;
   zoidfs_hint_create (&hint);
   setHint (&hint, ZOIDFS_SFP_OP, "GET");
   setHint (&hint, ZOIDFS_SFP_SFPID, sfp_id);

   zoidfs_sattr_t sattr;
   zoidfs_attr_t attr;
   zoidfs_setattr (handle, &sattr, &attr, &hint);

   boost::optional<std::string> response = getHint (&hint, ZOIDFS_SFP_VAL);
   ALWAYS_ASSERT(response);

   zoidfs_hint_free (&hint);
   return boost::lexical_cast<zoidfs::zoidfs_file_ofs_t> (*response);
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

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

struct SFPProvider
{
   virtual void write (const void * data, size_t bytes) = 0;

   virtual size_t getOfs () const = 0;

};


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

struct IOFSLSFP : public SFPProvider
{
   IOFSLSFP (MPI_Comm comm, const std::string & filename)
      : comm_ (comm)
   {
      MPI_Comm_rank (comm, &rank_);

      sfpid_ = gen ();
      sfpid_ <<= 32;
      sfpid_ |= gen ();

      MPI_Bcast (&sfpid_, sizeof(sfpid_), MPI_BYTE, 0, comm);

      if (!rank_)
      {
         int created;
         zoidfs_sattr_t attr;
         attr.mask = 0;
         check (zoidfs_create (0, 0, filename.c_str(), &attr, &handle_, &created,
                  0));

         createSFP (&handle_, sfpid_);
      }

      MPI_Bcast (&handle_, sizeof(handle_), MPI_BYTE, 0, comm);

      MPI_Barrier (comm);

      ALWAYS_ASSERT(getSFP (&handle_, sfpid_) == 0);

      MPI_Barrier (comm);
   }

   size_t getOfs () const
   { return getSFP (&handle_, sfpid_); }

   void write (const void * data, size_t bytes)
   {
      const zoidfs::zoidfs_file_ofs_t nextwrite = stepSFP (&handle_, sfpid_,
            bytes);
      const size_t memsizes = bytes;
      const zoidfs_file_size_t nextwritesize = bytes;

      check (zoidfs_write (&handle_, 1, &data, &memsizes, 
               1, &nextwrite, &nextwritesize,
               0));
   }
   
   virtual ~IOFSLSFP ()
   {
      MPI_Barrier (comm_);

      if (!rank_)
      {
         removeSFP (&handle_, sfpid_);
         zoidfs_finalize ();
      }
   }

   protected:
        int check (int ret) const
        {
           ALWAYS_ASSERT (ret == ZFS_OK);
           return ret;
        }

   protected:
        zoidfs::zoidfs_handle_t handle_;
        uint64_t sfpid_;
        MPI_Comm comm_;
        int rank_;
      
        static boost::mt19937 gen;
};

boost::mt19937 IOFSLSFP::gen (std::time(0) + getpid ());

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

struct MPISFP : public SFPProvider
{
   public:
      MPISFP (MPI_Comm comm, const std::string & filename)
      {
         checkMPI (MPI_File_open (comm,
                  const_cast<char*>(filename.c_str()),
                  MPI_MODE_RDWR|MPI_MODE_CREATE, MPI_INFO_NULL,
                  &file_));
         checkMPI (MPI_File_seek_shared (file_, 0, MPI_SEEK_SET));
      }

      virtual void write (const void * data, size_t bytes)
      {
         MPI_Status status;
         checkMPI (MPI_File_write_shared (file_, const_cast<void*>(data),
                  bytes, MPI_BYTE, &status));
      }

      virtual size_t getOfs () const
      {
         MPI_Offset ofs;
         checkMPI (MPI_File_get_position_shared (file_, &ofs));
         return ofs;
      }

      virtual ~MPISFP ()
      {
         MPI_File_close (&file_);
      }

   protected:
      inline int checkMPI (int ret) const
      {
         ALWAYS_ASSERT(ret == MPI_SUCCESS);
         return ret;
      }

   protected:
      MPI_File file_;

};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------


static std::string calcStats (std::vector<double> & data)
{
   typedef accumulator_set<double, features<tag::sum, tag::min, tag::max,
           tag::median, tag::moment<1>, tag::variance> > Accumulator;

   Accumulator acc;

   std::for_each( data.begin(), data.end(), boost::bind<void>( ref(acc), _1 ) );

   format f ("%i %d %d %d %d %d ");

   f  % accumulators::count(acc)
      % accumulators::min(acc)
      % accumulators::max(acc)
      % accumulators::moment<1>(acc)
      % accumulators::median(acc)
      % accumulators::variance(acc);
   return str(f);
}


double doTest (MPI_Comm comm, SFPProvider * provider, size_t blocksize, size_t
      blockcount)
{
   int rank;
   int commsize;
   MPI_Comm_rank (comm, &rank);
   MPI_Comm_size (comm, &commsize);

   boost::scoped_array<char> data (new char[blocksize]);
   std::fill (data.get(), data.get() + blocksize, rank);

   MPI_Barrier (comm);
   double start = MPI_Wtime ();

   for (size_t i=0; i<blockcount; ++i)
   {
      provider->write (data.get(), blocksize);
   }

   MPI_Barrier (comm);
   double stop = MPI_Wtime ();

   ALWAYS_ASSERT(provider->getOfs () == (blocksize * blockcount * commsize));

   return stop - start;
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

int main (int argc, char ** args)
{
   if (argc != 4)
   {
      cerr << args[0] << " filename blocksize blockcount\n";
      cerr << "    filename: file to write to\n";
      cerr << "    blocksize: size of each write (kb)\n";
      cerr << "    blockcount: number of blocks /per process/ to write\n";
      exit (1);
   }


   const std::string filename = args[1];
   const size_t blocksize = boost::lexical_cast<size_t> (args[2 ]) * 1024;
   const size_t blockcount = boost::lexical_cast<size_t> (args[3]);

   zoidfs_init ();
   MPI_Init (0,0);



   int ranks = 1;
   int worldsize;
   int worldrank;

   MPI_Comm_size (MPI_COMM_WORLD, &worldsize);
   MPI_Comm_rank (MPI_COMM_WORLD, &worldrank);

   MPI_Comm comm;


   while (true)
   {
      std::vector<double> iofsltimes;
      std::vector<double> mpitimes;

      MPI_Comm_split (MPI_COMM_WORLD, (worldrank < ranks ? 0 : MPI_UNDEFINED),
            worldrank, &comm);

      if (comm != MPI_COMM_NULL)
      {
         int commsize;
         int commrank;
         MPI_Comm_size (comm, &commsize);
         MPI_Comm_rank (comm, &commrank);

         for (size_t i=0; i<opt_loops; ++i)
         {
            boost::scoped_ptr<SFPProvider> test;

            if (!worldrank)
            {
               cout << "# Test (" << i+1 << ") iofsl:";
               cout.flush ();
            }

            MPI_Barrier (comm);

            test.reset (new IOFSLSFP (comm, filename.c_str()));
            iofsltimes.push_back (doTest (comm, test.get(), blocksize, blockcount));

            MPI_Barrier (comm);

            test.reset (new MPISFP (comm, filename.c_str()));
            mpitimes.push_back (doTest (comm, test.get(), blocksize, blockcount));

            if (!worldrank)
            {
               cout << iofsltimes.back () << " mpi:" << mpitimes.back () <<
                  endl;
               cout.flush ();
            }

            MPI_Barrier (comm);

         }

         if (!worldrank)
         {
            cout << "# commsize totalbytes [ rounds min max avg median"
               " variance ] \n";
            cout << commsize << " " << blocksize * blockcount * commsize <<
               " " << calcStats(iofsltimes) << calcStats(mpitimes) << endl;
            cout.flush ();
         }

         MPI_Comm_free (&comm);
      }

      if (ranks == worldsize)
         break;

      ranks = std::min (worldsize, ranks * 2);
   }

   MPI_Finalize ();
   zoidfs_finalize ();

   return 0;
}
