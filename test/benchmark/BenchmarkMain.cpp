#include "BenchmarkMain.hh"
#include <iostream>
#include <fstream>
#include <cstdio>
using namespace iofwd;
using namespace iofwdevent;
using namespace boost;
using namespace iofwd::service;
using namespace iofwd::extraservice;
using namespace std;
using namespace iofwdevent;
using namespace rpc;

void LogResult ( ofstream * f, char * s, double runTime, size_t outLen,
                char * filename)
{
  char * outStr = new char[strlen(s) + 45 + strlen(filename)];
  sprintf(outStr,"%s,%s,%10.8g,%i\n", s, filename, runTime, outLen);
  f->write(outStr, strlen(outStr));
  f->flush();
  fprintf(stderr,"%s,%s,%10.8g,%i\n", s, filename, runTime, outLen);
  delete[] outStr;
}

void control (char * UNUSED(address), char * UNUSED(config), char * inDataset, 
              char * outDataset, int readSize, int  runs, char* csvOutput)
{
  ofstream logfile;
  double start, end;
  logfile.open(csvOutput, ios::app);
  /* skip the first run */
  MPI_Barrier(MPI_COMM_WORLD);  
  MPI_Barrier(MPI_COMM_WORLD); 
  MPI_Barrier(MPI_COMM_WORLD);  
  MPI_Barrier(MPI_COMM_WORLD);     
  for (int i = 0; i < runs - 1; i++)
  {
     MPI_Barrier(MPI_COMM_WORLD); 
     start = MPI_Wtime();
     MPI_Barrier(MPI_COMM_WORLD);
     end = MPI_Wtime();
     LogResult(&logfile, "READ", end - start, readSize, inDataset);
     MPI_Barrier(MPI_COMM_WORLD); 
     start = MPI_Wtime();
     MPI_Barrier(MPI_COMM_WORLD);
     end = MPI_Wtime();
     LogResult(&logfile, "WRITE", end - start, readSize, outDataset);
  }
}
/*
void doLookup (iofwdclient::IOFWDClient * x, zoidfs::zoidfs_handle_t * outHandle, char * filename)
{
   zoidfs::zoidfs_op_hint_t op_hint;
   zoidfs::hints::zoidfs_hint_create(&op_hint);
   x->lookup (NULL, NULL, filename, outHandle, &op_hint);
}

void getHandles ( char * address, std::string opt_config, zoidfs::zoidfs_handle_t * outHandle, char * outfilename,
                   zoidfs::zoidfs_handle_t * inHandle, char * infilename)
{
   // Needed to autoregister services
   registerIofwdFactoryClients ();
   ServiceManager & man = ServiceManager::instance ();
   man.setParam ("config.configfile", opt_config);
   shared_ptr<iofwd::Net> netservice (man.loadService<iofwd::Net>("net"));
   SingleCompletion block;
   net::Net * net = netservice->getNet ();
   net::AddressPtr addr;
   net->lookup (address, &addr, block);
   block.wait ();
   iofwdclient::IOFWDClient * x  =  new iofwdclient::IOFWDClient (*(new iofwdclient::CommStream()),
                                                                  addr,
                                                                  (bool)true);
   doLookup(x, outHandle, outfilename);
   doLookup(x, inHandle, infilename);

   delete x;
}
*/
int main(int argc, char *argv[]) 
{
    int runs = 6;
    if (argc < 4)
      return -1;

    int n, rank, size, i; 
    char * address = argv[1];
    char * config = argv[2];
    char * inDataset = argv[3];
    char * outDataset = argv[4];
    int readSize = std::atoi(argv[5]);
    char * csvOutput = argv[6];    
    //zoidfs::zoidfs_handle_t outHandle;
    //zoidfs::zoidfs_handle_t inHandle;
    //getHandles (address, config, &outHandle, outDataset, &inHandle, inDataset);

    MPI::Init(argc, argv);           
    size = MPI::COMM_WORLD.Get_size(); 
    rank = MPI::COMM_WORLD.Get_rank(); 
    if (rank == 0)
      control (address, config, inDataset, outDataset, readSize, runs, csvOutput);
    else
      test (address, config, inDataset, outDataset, readSize, runs);
    
    MPI::Finalize(); 
}

