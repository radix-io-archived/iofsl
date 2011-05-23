#include <mpi.h>
#include <zoidfs.h>

#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <boost/format.hpp>
#include <iostream>

namespace po = boost::program_options;

using boost::format;

using namespace std;

static int rank;
static int size;

static unsigned int opt_loops = 1;
static std::string  opt_filename;

inline int check (int ret, const char * name)
{
   if (ret == zoidfs::ZFS_OK)
      return ret;

   cerr << format("Zoidfs call %s failed on rank %i: %i\nAborting\n")
      % name % rank % ret;

   return ret;
}


int main (int argc, char ** args)
{

   MPI_Init (0, 0);

   zoidfs::zoidfs_init ();

   MPI_Comm_size (MPI_COMM_WORLD, &size);
   MPI_Comm_rank (MPI_COMM_WORLD, &rank);

   po::options_description desc("Supported options");
   desc.add_options()
      ("loops", po::value<unsigned int>(&opt_loops), "Number of iterations")
      ("file", po::value<std::string>(&opt_filename), "Filename to create")
      ;
   po::variables_map vm;
   po::parsed_options parsed =
      po::command_line_parser(argc, args).options(desc).run();
   po::store(parsed, vm);
   po::notify(vm);

   if (opt_filename.empty ())
   {
      cerr << "Need filename!\n";
      MPI_Abort (MPI_COMM_WORLD, 1);
   }

   if (!rank)
   {
      cout << format("Starting test with file '%s', %i ranks, %i"
            " iterations\n") % opt_filename % size % opt_loops;
      cout.flush ();
   }

   for (size_t i=0; i<opt_loops; ++i)
   {
      MPI_Barrier (MPI_COMM_WORLD);

      int created;
      zoidfs::zoidfs_sattr_t attr;
      zoidfs::zoidfs_handle_t handle;
      attr.mask = 0;
      check (zoidfs::zoidfs_create (0, 0, opt_filename.c_str(), &attr, &handle,
                                    &created, ZOIDFS_NO_OP_HINT),
             "zoidfs_create");

      created = (created ? 1 : 0);

      int sum;
      MPI_Reduce (&created, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
      if (!rank)
      {
         if (sum != 1)
         {
            cerr << str(format("Number of processes returning created = 1 was"
                     " != 1! (%i)\nAborting!\n") % sum);
            MPI_Abort (MPI_COMM_WORLD, 1);
         }
      }

      if (rank)
      {
         MPI_Bcast (&handle.data, sizeof(handle.data), MPI_BYTE, 0,
               MPI_COMM_WORLD);
      }
      else
      {
         zoidfs::zoidfs_handle_t master = handle;
         MPI_Bcast (&master.data, sizeof(master.data), MPI_BYTE, 0,
               MPI_COMM_WORLD);
         if (memcmp (&master.data, &handle.data, sizeof(handle.data)))
         {
            cerr << format("Rank %i got different handle!\n") % rank;
            MPI_Abort (MPI_COMM_WORLD, 1);
            return 1;
         }
      }

      MPI_Barrier (MPI_COMM_WORLD);

      if (!rank)
      {
         check (zoidfs::zoidfs_remove(0, 0, opt_filename.c_str(),
                              0, ZOIDFS_NO_OP_HINT),
               "zoidfs_remove");

         cout << '.';
         cout.flush ();
      }
   }

   zoidfs::zoidfs_finalize ();

   MPI_Finalize ();
   return 0;
}
