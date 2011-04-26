#include "Test.h"

void lookupInput (zoidfs_handle_t * outHandle, char * filename)
{
   fprintf (stderr, "LOOKUP %s\n", filename);
   zoidfs_lookup (NULL, NULL, filename, outHandle, ZOIDFS_NO_OP_HINT);
   fprintf (stderr, "LOOKUP\n");       
}

void test (char * UNUSED(address), char * UNUSED(config), char * inDataset, char * outDataset, 
           int readSize, int runs)
{
  int ret;
  zoidfs_init();
  fprintf (stderr, "Start\n");
  zoidfs_handle_t outHandle;
  zoidfs_handle_t inHandle;
  lookupInput (&outHandle, outDataset);
  lookupInput (&inHandle, inDataset);
  int i;  
  for (i = 0; i < runs; i++)
  {    

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

      /* Test Start Barrier */ 
      MPI_Barrier(MPI_COMM_WORLD);
      ret = zoidfs_read (&inHandle, mem_count, (void **)mem_starts_write, mem_sizes, 
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
      ret = zoidfs_write (&outHandle, mem_count, (const void **)mem_starts_write, 
                    mem_sizes, file_count, file_starts, file_sizes, 
                    ZOIDFS_NO_OP_HINT); 
      MPI_Barrier(MPI_COMM_WORLD);
       for(_i = 0 ; _i < mem_count ; _i++) 
       { 
           free(mem_starts_write[_i]); 
       } 
      

  }
}

