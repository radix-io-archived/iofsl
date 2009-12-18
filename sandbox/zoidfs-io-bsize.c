#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd_config.h"

#define NAMESIZE 255
#define BUFSIZE (8 * 1024 * 1024)

int safe_read (const char * fullpath, 
      const zoidfs_handle_t * handle /* in:ptr */,
                size_t mem_count /* in:obj */,
                void * mem_starts[] /* out:arr2d:size=+1:zerocopy */,
                const size_t mem_sizes[] /* in:arr:size=-2 */,
                size_t file_count /* in:obj */,
                const uint64_t file_starts[] /* in:arr:size=-1 */,
                uint64_t file_sizes[] /* inout:arr:size=-2 */)
{
   assert (fullpath); 
   return zoidfs_read (handle, mem_count, mem_starts, 
         mem_sizes, file_count, file_starts, file_sizes, ZOIDFS_NO_OP_HINT); 
}

int safe_write (const char * fullpath, 
      const zoidfs_handle_t * handle /* in:ptr */,
                 size_t mem_count /* in:obj */,
                 const void * mem_starts[] /* in:arr2d:size=+1:zerocopy */,
                 const size_t mem_sizes[] /* in:arr:size=-2 */,
                 size_t file_count /* in:obj */,
                 const uint64_t file_starts[] /* in:arr:size=-1 */,
                 uint64_t file_sizes[] /* inout:arr:size=-2 */)
{
   assert (fullpath); 
   return zoidfs_write (handle, mem_count, mem_starts, mem_sizes, file_count, 
         file_starts, file_sizes, ZOIDFS_NO_OP_HINT); 
}


void run_io(char * mpt)
{
   char fullpath_filename[NAMESIZE];
   zoidfs_handle_t handle; 
   int created; 
   zoidfs_sattr_t attr; 
   const unsigned int mb = 1024*1024; 
   const unsigned int filesize = 64*mb; 
   const unsigned int maxseg = 32; 
   const unsigned int maxsegsize = filesize;
   const unsigned int maxmem = 2*filesize; 
   const unsigned int rounds = 100; 
   const unsigned int firstskip = 4*mb; 
   const unsigned int maxhole = 4*mb; 
   void * mem; 
   uint64_t * ofs; 
   uint64_t * size; 
   unsigned int round; 

   assert (firstskip <= filesize); 

   attr.mask = 0; 

   /* create fullpath_filename */
   snprintf(fullpath_filename, NAMESIZE, "%s/test-zoidfs-file-full", mpt); 
   zoidfs_create (0, 0, fullpath_filename, &attr, &handle, &created, ZOIDFS_NO_OP_HINT); 

   mem = malloc (maxmem); 
   ofs = malloc (sizeof (uint64_t) * maxseg); 
   size = malloc (sizeof (uint64_t) * maxseg); 

   for (round=0; round<rounds; ++round)
   {
      unsigned int i; 
      uint64_t curofs = random () % firstskip; 
      uint64_t cursize; 
      size_t totalsize = 0;

      /* generate access list */ 
      const unsigned int segments = random () % maxseg; 
      unsigned int segcount = 0; 

      int ret; 
      char fill; 

      for (i=0; i<segments; ++i)
      {
         cursize = random () % maxsegsize; 
         curofs += random () % maxhole; 

         if (curofs > filesize)
            curofs = filesize; 

         if (curofs + cursize > filesize)
            cursize = filesize - curofs; 

         if (curofs && cursize)
         {
            ofs[segcount] = curofs; 
            size[segcount] = cursize; 
            totalsize += cursize; 
            ++segcount; 
         }
      }

      fill = random() % 256; 
      memset (mem, fill, totalsize); 

      ret = safe_write (fullpath_filename, &handle, 1, (const void **) &mem, &totalsize, segcount,
            ofs, size);

      ret = safe_read (fullpath_filename, &handle, 1, &mem, &totalsize, segcount, 
            ofs, size); 

      for (i=0; i<totalsize; ++i)
      {
         if ( ((const char *) mem)[i] != fill)
         {
            char buf[255]; 
            snprintf (buf, sizeof(buf), "Problem in data validation! Expected %i, got %i!",
                  (int) fill, (int) ((const char *)mem)[i]); 
            break; 
         }
      }
   }

   free (size); 
   free (ofs); 
   free (mem); 
}

int main(int argc, char ** argv)
{
   if(argc < 2)
   {
    fprintf(stderr, "incorrect cmd line args!\n");
    return -1;
   }

   zoidfs_init();

   run_io(argv[1]);

   zoidfs_finalize();

    return 0;
}
