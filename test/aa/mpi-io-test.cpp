#include <iostream>
#include <mpi.h>
#include <vector>

#include <fstream>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/format.hpp>
#include <boost/scoped_array.hpp>

using namespace std;
using namespace boost;
using namespace boost::accumulators;

static int worldsize;
static int worldrank;

static double opt_mean = 512*1024;  // mean of block size distribution
static double opt_sigma = 10;       // sigma of block size distribution
static size_t opt_loops = 16;       // number of repeats for a test
static size_t opt_blocks = 64;      // number of blocks per process per test
static string opt_filename;         // File we're writing to


static mt19937 randomgen;

static void seedRandom ()
{
   ifstream rand("/dev/random");
   uint32_t seed;
   rand.read(reinterpret_cast<char*>(&seed),sizeof(seed));
   rand.close();
   randomgen.seed (seed);
}

static void dumpStats (size_t ranks, size_t datasize,
      const std::vector<double> & times)
{
   static format f("%i %i %lu %f %f\n");
   if (ranks == 1)
   {
      cout << "ranks samples bytes min mean\n";
      cout << "===================================\n";
   }
   accumulator_set<double, features<tag::min, tag::mean>  > acc;
   std::for_each (times.begin(), times.end(), boost::bind<void>(ref(acc), _1));
   f % ranks
      % accumulators::count(acc)
      % datasize
      % accumulators::min(acc)
      % accumulators::mean(acc);
   cout << str(f);
}

int checkMPI (int ret)
{
   if (ret == MPI_SUCCESS)
      return ret;

   cerr << "MPI Call failed!";
   MPI_Abort (MPI_COMM_WORLD, 1);
   return ret;
}

double dumpBlocks (MPI_Comm comm, MPI_File handle)
{
   const size_t maxblock = *std::max_element (blocksizes.begin(),
         blocksizes.end());
   boost::scoped_array<char> mem (new char[maxblock]);
   std::fill (&mem[0], &mem[maxblock], 0);

   MPI_Barrier (comm);
   const double start = MPI_Wtime ();

   BOOST_FOREACH(const size_t thisblock, blocksizes)
   {
      MPI_Status status;
      checkMPI (MPI_File_write_shared (handle, &mem[0], thisblock, MPI_BYTE,
               &status));

   }

   MPI_Barrier (comm);
   const double stop = MPI_Wtime ();
   return stop - start;
}

double runSingleTest (MPI_Comm comm, const std::vector<size_t> & blocksizes)
{
   int commrank;
   int commsize;

   MPI_Comm_rank (comm, &commrank);
   MPI_Comm_size (comm, &commsize);

   MPI_Barrier (comm);

   MPI_File handle;
   checkMPI(MPI_File_open (comm, const_cast<char*>(opt_filename.c_str()),
            MPI_MODE_WRONLY|MPI_MODE_CREATE,
            MPI_INFO_NULL, &handle));

   const double dumptime = dumpBlocks (comm, handle, blocksizes);

   if (!commrank)
   {
      MPI_File_delete (const_cast<char*>(opt_filename.c_str()),
            MPI_INFO_NULL);
   }

   MPI_Barrier (comm);
   return dumptime;
}


size_t runTest (MPI_Comm comm, std::vector<double> & res)
{

   // === calculate random block size distribution ===
   normal_distribution<double> distrib (opt_mean, opt_sigma);
   std::vector<size_t> blocksizes;
   blocksizes.reserve (opt_blocks);

   for (size_t i =0; i<opt_blocks; ++i)
   {
      const size_t thisblock = distrib (randomgen);
      blocksizes.push_back (thisblock);
   }

   res.clear ();
   for (size_t i=0; i<opt_loops; ++i)
   {
      res.push_back (runSingleTest (comm, blocksizes));
   }

   const size_t transfersize = std::accumulate (blocksizes.begin(),
         blocksizes.end(), 0);
}

int main (int argc, char ** args)
{
   seedRandom ();

   opt_filename = "/tmp/testfile";

   MPI_Init (0,0);

   MPI_Comm_size (MPI_COMM_WORLD, &worldsize);
   MPI_Comm_rank (MPI_COMM_WORLD, &worldrank);

   ssize_t nodes = 1;

   std::vector<double> results;

   do
   {
      MPI_Comm comm;

      MPI_Comm_split (MPI_COMM_WORLD,
            (worldrank < nodes ? 0 : MPI_UNDEFINED),
            worldrank,
            &comm);

      if (comm != MPI_COMM_NULL)
      {
         runTest (comm, results);
         MPI_Comm_free (&comm);
      }

      if (nodes == worldsize)
         break;

      nodes = std::min (static_cast<ssize_t>(worldsize), nodes * 2);
   } while (nodes <= worldsize);

   MPI_Finalize ();
   return EXIT_SUCCESS;
}

