#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <getopt.h>

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd_config.h"

#define NAMESIZE 255
#define BUFSIZE (8 * 1024 * 1024)
#define GB (1024 * 1024 * 1024)
#define MB (1024 * 1024)
#define KB 1024

static size_t filesize = 64 * MB;
static unsigned int maxseg = 1024;
static size_t maxsegsize = 64 * MB;
static unsigned int rounds = 100;
static char debug = 0;

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

size_t size_convert(char * val)
{
    int len = strlen(val);
    
    switch(val[len - 1])
    {
        case 'K':
            return ((size_t)atoi(val)) * KB;
            break;
        case 'M':
            return ((size_t)atoi(val)) * MB;
            break;
        case 'G':
            return ((size_t)atoi(val)) * GB;
            break;
        default:
            return ((size_t)atoi(val));
            break;
    }

    return 0;
}
int parse_options(int argc, char * argv[])
{
    extern char *optarg;
    char flags[] = "f:s:S:r:Rdh";
    int option_index = 0;
    int cur_opt = 0;
    static struct option long_options[] = {
        {"filesize", 1, 0, 'f'},
        {"maxsegments", 1, 0, 's'},
        {"maxsegmentsize", 1, 0, 'S'},
        {"rounds", 1, 0, 'r'},
        {"random", 0, 0, 'R'},
        {"debug", 0, 0, 'd'},
        {"help", 0, 0, 'h'},
        {0, 0, 0, 0}
    };

    /* look at command line arguments */
    while ((cur_opt = getopt_long(argc, argv, flags, long_options, &option_index)) != -1)
    {
        switch (cur_opt)
        {
            case 'f':
                filesize = size_convert(optarg);
                break;
            case 's':
                maxseg = (unsigned int)atoi(optarg);
                break;
            case 'S':
                maxsegsize = size_convert(optarg);
                break;
            case 'r':
                rounds = (unsigned int)atoi(optarg);
                break;
            case 'R':
                srand(time(0));
                break;
            case 'd':
                debug = 1;
                break;
            case 'h':
                fprintf(stderr, "zoidfs-io-bsize: random zoidfs io test program\n");
                fprintf(stderr, "\t--filesize=<val>, -f <val>: the max size of the file (in K or M)\n");
                fprintf(stderr, "\t--maxsegments=<val>, -s <val>: the max number of segments in the file\n");
                fprintf(stderr, "\t--maxsegmentsize=<val>, -S <val>: the max size of a single segment (in K or M)\n");
                fprintf(stderr, "\t--rounds=<val>, -r <val>: the number of tests to run\n");
                fprintf(stderr, "\t--random=<val>, -R <val>: seed the random number generator(current time is used)\n");
                fprintf(stderr, "\t--debug, -d: enable debug print statements\n");
                fprintf(stderr, "\t--help, -h: print this message\n");

                exit(1);
            default:
                break;
        }
        option_index = 0;
    }

    return 0;
}

void run_io(char * mpt)
{
   char fullpath_filename[NAMESIZE];
   zoidfs_handle_t handle; 
   int created; 
   zoidfs_sattr_t attr; 
   void * mem; 
   uint64_t * ofs; 
   uint64_t * size; 
   unsigned int round; 
   const unsigned int maxmem = 2*filesize; 

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
      uint64_t curofs = 0; 
      uint64_t cursize; 
      size_t totalsize = 0;

      /* generate access list */ 
      const unsigned int segments = random () % maxseg; 
      unsigned int segcount = 0; 

      int ret; 
      char fill; 

      if(debug)
        fprintf(stderr, "io operation #%i:\n", round);
      for (i=0; i<segments; ++i)
      {
         cursize = random () % maxsegsize; 

         if (curofs > filesize)
         {
            curofs = filesize; 
            break;
         }

         if (curofs + cursize > filesize)
         {
            cursize = filesize - curofs;
            break;
         } 

         if (cursize)
         {
            if(debug)
                fprintf(stderr, " seg #%i : ofs = %lu, size = %lu\n", segcount, curofs, cursize);
            ofs[segcount] = curofs; 
            size[segcount] = cursize; 
            totalsize += cursize; 
            curofs += cursize;
            ++segcount; 
         }
      }

      if(debug)
        fprintf(stderr, " total size = %lu, total segs = %i\n", totalsize, segcount);

      fill = (random() % 255) + 1;
      memset (mem, fill, maxmem); 

      ret = safe_write (fullpath_filename, &handle, 1, (const void **) &mem, &totalsize, segcount,
            ofs, size);

      /* clear out the buffer */
      memset(mem, 0, maxmem);
      ret = safe_read (fullpath_filename, &handle, 1, &mem, &totalsize, segcount, 
            ofs, size); 

      int dvfailed = 0;
      for (i=0; i<totalsize; ++i)
      {
         if ( ((const char *) mem)[i] != fill)
         {
            char buf[255]; 
            snprintf (buf, sizeof(buf), "Problem in data validation! Expected %i, got %i!",
                  (int) fill, (int) ((const char *)mem)[i]); 
            dvfailed = 1;
            break; 
         }
      }

      if(!dvfailed)
      {
        if(debug)
            fprintf(stderr, "Data validation passed.\n");
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

   parse_options(argc, argv);

   zoidfs_init();

   run_io(argv[1]);

   zoidfs_finalize();

    return 0;
}
