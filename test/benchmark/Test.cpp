#include "iofwdutil/IofwdutilLinkHelper.hh"
#include "Test.hh"
using namespace iofwd;
using namespace iofwdevent;
using namespace boost;
using namespace iofwd::service;
using namespace iofwd::extraservice;
using namespace std;
using namespace iofwdevent;
using namespace rpc;
iofwdclient::IOFWDClient * iofslSetup( char * address, std::string opt_config)
{
   // Needed to autoregister services
   registerIofwdFactoryClients ();
   registerIofwdutilFactoryClients (); 
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
   return x;
}


//void createOutput (iofwdclient::IOFWDClient * x, zoidfs::zoidfs_handle_t * outHandle, char * filename)
//{
////    registerIofwdFactoryClients (); 
//    size_t ret = 0;
//    int created;
//    struct timeval now; 
//    zoidfs::zoidfs_sattr_t sattr; 
//    zoidfs::zoidfs_op_hint_t op_hint;
//    zoidfs::hints::zoidfs_hint_create(&op_hint);
////    sattr.mask = ZOIDFS_ATTR_MODE  | ZOIDFS_ATTR_UID | 
////                 ZOIDFS_ATTR_GID   | ZOIDFS_ATTR_ATIME | 
////                 ZOIDFS_ATTR_MTIME; 
////    sattr.mode = 0777; 
////    sattr.uid = getuid(); 
////    sattr.gid = getgid(); 
////    gettimeofday(&now, NULL); 
////    sattr.atime.seconds = now.tv_sec; 
////    sattr.atime.nseconds = now.tv_usec; 
////    sattr.mtime.seconds = now.tv_sec; 
////    sattr.mtime.nseconds = now.tv_usec; 
//    ret = x->lookup (NULL, NULL, filename, outHandle, &op_hint); 
//}

void lookupInput (iofwdclient::IOFWDClient * x, zoidfs::zoidfs_handle_t * outHandle, char * filename)
{
   zoidfs::zoidfs_op_hint_t op_hint;
   zoidfs::hints::zoidfs_hint_create(&op_hint);
   x->lookup (NULL, NULL, filename, outHandle, &op_hint);
       
}

void test (char * address, char * config, char * inName, char * outName,
           int readSize, int runs)
{
  fprintf (stderr, "Welcome Home\n");
  int ret;
  zoidfs::zoidfs_handle_t outHandle;
  zoidfs::zoidfs_handle_t inHandle;
  iofwdclient::IOFWDClient * x = iofslSetup (address, std::string(config));
  fprintf(stderr, "First Lookup\n");
  lookupInput (x, &outHandle, outName);
  fprintf(stderr, "Second Lookup\n");
  lookupInput (x, &inHandle, inName);
  for (int i = 0; i < runs; i++)
  {    
//	  fprintf(stderr, "First Lookup\n");
//	  lookupInput (x, &outHandle, outName);
//	  fprintf(stderr, "Second Lookup\n");
//	  lookupInput (x, &inHandle, inName);
//      MPI_Barrier(MPI_COMM_WORLD);
//      MPI_Barrier(MPI_COMM_WORLD);
//     MPI_Barrier(MPI_COMM_WORLD);
//     MPI_Barrier(MPI_COMM_WORLD);

      fprintf (stderr, "Run Number: %i\n",i);
      int  _N = 1;
      size_t _BSIZE = readSize;
      size_t mem_sizes[_N]; 
      size_t _foff = 0; 
      size_t mem_count, file_count; 
      uint64_t file_sizes[_N], file_starts[_N]; 
      void *mem_starts_write[_N]; 
      size_t _i = 0; 
      mem_count = _N; 
      file_count = _N; 
      for(_i = 0 ; _i < mem_count ; _i++) 
      { 
      mem_starts_write[_i] = malloc(_BSIZE); 
      file_sizes[_i] = mem_sizes[_i] = _BSIZE; 
      file_starts[_i] = _foff; 
      _foff += _BSIZE; 
      } 

      // Test Start Barrier 
      MPI_Barrier(MPI_COMM_WORLD);
//      fprintf (stderr, "Starting Read\n");
      ret = x->read (&inHandle, mem_count, (void **)mem_starts_write, mem_sizes, 
                   file_count, file_starts, file_sizes, ZOIDFS_NO_OP_HINT);
      MPI_Barrier(MPI_COMM_WORLD);


      _foff = 0;
      for(_i = 0 ; _i < mem_count ; _i++) 
      { 
      file_sizes[_i] = mem_sizes[_i] = _BSIZE; 
      file_starts[_i] = _foff; 
      _foff += _BSIZE; 
      } 

      MPI_Barrier(MPI_COMM_WORLD);    
      ret = x->write (&outHandle, mem_count, (const void **)mem_starts_write, 
                    mem_sizes, file_count, file_starts, file_sizes, 
                    ZOIDFS_NO_OP_HINT); 
      MPI_Barrier(MPI_COMM_WORLD);
       for(_i = 0 ; _i < mem_count ; _i++) 
       { 
           free(mem_starts_write[_i]); 
       } 


  }
}

