#include <iostream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_array.hpp>
#include <mpi.h>
#include <sstream>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/density.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/foreach.hpp>
#include <algorithm>
#include <boost/bind.hpp>

using namespace boost::accumulators;
using namespace std;
using namespace boost::program_options;
using namespace boost;


enum { KB = 1024, MB = 1024*KB };

static size_t opt_groupsize = 64;
static size_t opt_blocksize = 4*MB;
static bool opt_collective = false;
static size_t opt_count = 1024;
static size_t opt_loops;
static std::string opt_mapping;
static bool opt_verbose;
static bool opt_noworld;

struct MyGenerator
{
   MyGenerator (size_t group, int rank, int size)
      : val_(group+rank+size)
   {
   }

   inline unsigned char operator () ()
   {
      return val_;
   }

   unsigned char val_;
};

template <typename ACC>
void reduceData (MPI_Comm comm, const std::vector<double> & val, ACC & acc)
{
   int commrank;
   int commsize;
   MPI_Comm_size (comm, &commsize);
   MPI_Comm_rank (comm, &commrank);

   unsigned int samples = val.size();
   unsigned int maxsamples;
   MPI_Allreduce (&samples, &maxsamples, 1, MPI_UNSIGNED, MPI_MAX, comm);

   MPI_Request req;
   MPI_Isend (const_cast<void*>(static_cast<const void*>(&val[0])),
         val.size(), MPI_DOUBLE, 0, 0, comm, &req);
   
   if (!commrank)
   {
      vector<double> recvbuffer;
      recvbuffer.resize (maxsamples);
      for (int i=0; i<commsize; ++i)
      {
         MPI_Status status;
         MPI_Recv (&recvbuffer[0], recvbuffer.size(), MPI_DOUBLE,
               MPI_ANY_SOURCE, 0, comm, &status);

         int count;
         MPI_Get_count (&status, MPI_DOUBLE, &count);

         std::for_each (recvbuffer.begin(), recvbuffer.begin()+count,
               boost::bind(ref(acc), _1));
      }
   }

   MPI_Wait (&req, MPI_STATUS_IGNORE);

   
}

static void calcStats (MPI_Comm comm, bool read, size_t iter, size_t groupnum,
      int rank, int , const std::vector<double> & data)
{
   int worldrank;
   int worldsize;
   MPI_Comm_rank (MPI_COMM_WORLD, &worldrank);
   MPI_Comm_size (MPI_COMM_WORLD, &worldsize);

   typedef accumulator_set<double, features<tag::sum, tag::min, tag::max,
      tag::median, tag::moment<1>, tag::variance> > Accumulator;
   Accumulator acc;

   // Calculate local statistics
   std::for_each (data.begin(), data.end(), boost::bind(ref(acc), _1));

   if (!opt_noworld)
      MPI_Barrier (MPI_COMM_WORLD);

   boost::format f("%c %c %i %i %i %i %i %.12f %.12f %.12f %.12f %.12f %.12f\n");
   if (!worldrank)
   {
      if (opt_verbose)
      {
         cout << "R/W type iter worldrank groupnum grouprank samples "
            "min max avg median sum variance\n";
      }
      cout.flush ();
   }

   if (!opt_noworld)
      MPI_Barrier (MPI_COMM_WORLD);

   f         % (read ? 'R' : 'W')
             % 'I'
             % iter
             % worldrank
             % groupnum
             % rank
             % accumulators::count(acc)
             % accumulators::min(acc)
             % accumulators::max(acc)
             % accumulators::moment<1>(acc)
             % accumulators::median(acc)
             % accumulators::sum(acc)
             % accumulators::variance(acc);
   cout << str(f);
   cout.flush ();

   if (opt_groupsize != 1)
   {
      // clear statistics
      acc = Accumulator ();
      reduceData (comm, data, acc);

      if (!rank)
      {
         f  % (read ? 'R' : 'W')
            % 'G'
            % iter
            % worldrank
            % groupnum
            % rank
            % accumulators::count(acc)
            % accumulators::min(acc)
            % accumulators::max(acc)
            % accumulators::moment<1>(acc)
            % accumulators::median(acc)
            % accumulators::sum(acc)
            % accumulators::variance(acc);
         cout << str(f);
         cout.flush ();
      }

      if (!opt_noworld)
         MPI_Barrier (MPI_COMM_WORLD);
   }

   if (opt_groupsize != worldsize && !opt_noworld)
   {
      // clear statistics
      acc = Accumulator ();
      reduceData (MPI_COMM_WORLD, data, acc);

      if (!worldrank)
      {
         f  % (read ? 'R' : 'W')
            % 'W'
            % iter
            % worldrank
            % groupnum
            % rank
            % accumulators::count(acc)
            % accumulators::min(acc)
            % accumulators::max(acc)
            % accumulators::moment<1>(acc)
            % accumulators::median(acc)
            % accumulators::sum(acc)
            % accumulators::variance(acc);
         cout << str(f);
         cout.flush ();
      }

      if (!opt_noworld)
         MPI_Barrier (MPI_COMM_WORLD);
   }
 }


static void runTest (size_t iter, MPI_Comm comm, size_t groupnum,
      const std::string & filename)
{

   int commrank;
   int commsize;
   int worldrank;

   int (*writefunc) (MPI_File, MPI_Offset, void *, int, MPI_Datatype,
         MPI_Status *);
   int (*readfunc) (MPI_File, MPI_Offset, void *, int, MPI_Datatype,
         MPI_Status *);

   writefunc = (opt_collective ? MPI_File_write_at_all : MPI_File_write_at);
   readfunc = (opt_collective ? MPI_File_read_at_all : MPI_File_read_at);


   MPI_Comm_rank (MPI_COMM_WORLD, &worldrank);
   MPI_Comm_rank (comm, &commrank);
   MPI_Comm_size (comm, &commsize);


   scoped_array<unsigned char> data (new unsigned char[opt_blocksize]);
   MyGenerator gen (groupnum, commrank, commsize);
   std::generate (&data[0], &data[opt_blocksize], gen);

   MPI_Barrier (MPI_COMM_WORLD);
   if (!worldrank && opt_verbose)
   {
      cout << "# Write testing...\n";
   }
   MPI_Barrier (MPI_COMM_WORLD);


   MPI_File file = MPI_FILE_NULL;
   MPI_File_open (comm, const_cast<char*>(filename.c_str()),
         MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &file);

   if (file == MPI_FILE_NULL)
   {
      MPI_Abort (MPI_COMM_WORLD, 1);
   }

   MPI_Barrier (comm);
   MPI_File_set_size (file, 0);
   MPI_Barrier (comm);


   vector<double> vals;
   vals.reserve(opt_count);

   for (size_t i=0; i<opt_count; ++i)
   {
      MPI_Status status;
      MPI_Offset offset = opt_blocksize * ((opt_count * commrank) + i);
      const double start = MPI_Wtime ();
      writefunc (file, offset, &data[0], opt_blocksize, MPI_BYTE,
            &status);
      const double stop = MPI_Wtime ();
      vals.push_back (stop-start);
   }
   MPI_File_close (&file);

   calcStats (comm, true, iter, groupnum, commrank, commsize, vals);


   MPI_Barrier (MPI_COMM_WORLD);
   if (!worldrank && opt_verbose)
   {
      cout << "# Read testing...\n";
   }
   MPI_Barrier (MPI_COMM_WORLD);

   vals.resize(opt_count);

   MPI_File_open (comm, const_cast<char*>(filename.c_str()),
         MPI_MODE_RDONLY, MPI_INFO_NULL, &file);

   if (file == MPI_FILE_NULL)
   {
      MPI_Abort (MPI_COMM_WORLD, 1);
   }


   scoped_array<unsigned char> readdata (new unsigned char[opt_blocksize]);
   vals.clear ();

   for (size_t i=0; i<opt_count; ++i)
   {
      MPI_Status status;
      MPI_Offset offset = opt_blocksize * ((opt_count * commrank) + i);
      const double start = MPI_Wtime ();
      readfunc (file, offset, &readdata[0], opt_blocksize, MPI_BYTE,
            &status);
      const double stop = MPI_Wtime ();
      vals.push_back (stop-start);
      if (!equal (&readdata[0], &readdata[opt_blocksize], &data[0]))
      {
         cerr << "Data validation error!\n";
         MPI_Abort (MPI_COMM_WORLD, 1);
      }
   }
   MPI_File_close (&file);

   calcStats (comm, false, iter, groupnum, commrank, commsize, vals);
}

std::string lookupFile (MPI_Comm comm, const std::string & mapfile, unsigned
      int groupnum)
{
   char buf[255];
   memset (&buf[0], 0, sizeof(buf));

   int commrank;
   MPI_Comm_rank (comm, &commrank);

   buf[0] = 0;

   if (!commrank)
   {
      MPI_File f;
      MPI_File_open (MPI_COMM_SELF, const_cast<char*>(mapfile.c_str()),
            MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
      MPI_File_set_errhandler (f, MPI_ERRORS_ARE_FATAL);
      MPI_Offset size = 0;
      MPI_File_get_size (f, &size);
      boost::scoped_array<char> data (new char[size+1]);
      MPI_File_read (f, &data[0], size, MPI_BYTE, MPI_STATUS_IGNORE);
      MPI_File_close (&f);

      data[size] = 0;
      istringstream in (&data[0]);
      
      std::string line;
      for (size_t i=0; i<groupnum; ++i)
      {
         getline (in, line);
         if (!in)
            break;
      }
      if (in)
      {
         getline (in, line);
         if (line.size() < sizeof(buf))
         {
            copy (line.begin(), line.end(), &buf[0]);
         }
         buf[line.size()]=0;
      }
   }

   MPI_Bcast (&buf[0], sizeof(buf), MPI_BYTE, 0, comm);
   return std::string (buf);
}

int main (int argc, char ** args)
{
   MPI_Init (0, 0);

   int commsize;
   int commrank;

   MPI_Comm_size (MPI_COMM_WORLD, &commsize);
   MPI_Comm_rank (MPI_COMM_WORLD, &commrank);

   options_description desc ("Options");
   desc.add_options ()
      ("help", "Show program usage")
      ("collective", "Use collective I/O")
      ("groupsize", value<size_t>(&opt_groupsize)->default_value(64),
         "Number of ranks in a group")
       ("count", value<size_t>(&opt_count)->default_value(1024),
        "Number of blocks to be written /per rank/")
       ("blocksize", value<size_t>(&opt_blocksize)->default_value(4*KB),
        "Size (in KB) of block")
       ("mapping", value<std::string>(&opt_mapping),
          "File describing group->file mapping")
       ("loop", value<size_t>(&opt_loops)->default_value(1),
        "Number of times to run each test")
       ("verbose", "Be more verbose")
       ("noworld", "Don't calculate MPI_COMMWORLD number")
    ;

   positional_options_description p;
   p.add("mapping", -1);

   variables_map vm;
   store(command_line_parser(argc, args).options(desc).positional(p).run(), vm);
   notify(vm);

   do
   {
      opt_verbose = vm.count("verbose");
      opt_collective = vm.count("collective");
      opt_noworld = vm.count("noworld");

      if (vm.count("mapping") != 1)
      {
         if (!commrank)
            cerr << "Error: Need exactly one mapping file!\n";
         break;
      }

      if (commsize % opt_groupsize)
      {
         if (!commrank)
            cerr << "Number of processes is not a multiple of groupsize!\n";
         break;
      }

      if (!commrank)
      {
         cout << "Configuration: \n";
         cout << str(format("Collective      : %i\n") % opt_collective);
         cout << str(format("Groupsize       : %i\n") % opt_groupsize);
         cout << str(format("Count           : %i\n") % opt_count);
         cout << str(format("Blocksize       : %i\n") % opt_blocksize);
         cout << str(format("Processes       : %i\n") % commsize);
         cout << str(format("Total bytes     : %i\n") %
               (MPI_Offset(opt_blocksize)*opt_count*commsize));
      }

      // Split into groups
      MPI_Comm newcomm = MPI_COMM_NULL;
      const unsigned int groupnum = commrank / opt_groupsize;
      MPI_Comm_split (MPI_COMM_WORLD, groupnum, commrank,
            &newcomm);

      const std::string destfile = lookupFile (newcomm, opt_mapping, groupnum);
      if (destfile.empty())
      {
         cerr << "Problem reading mapping file...\n";
         MPI_Abort (MPI_COMM_WORLD, 1);
      }

      cout << str(format("# Group %i: filename '%s'\n")
            % groupnum % destfile);

      for (size_t l = 0; l<opt_loops; ++l)
      {
         runTest (l, newcomm, groupnum, destfile);
      }

      MPI_Comm_free (&newcomm);
   } while (false);


   MPI_Finalize ();
}
