#ifndef IOFWD_POSIX_PERSIST_H
#define IOFWD_POSIX_PERSIST_H

#include <stdint.h>
#include "zoidfs.h"

enum { PERSIST_HANDLE_MAXNAME = 256 };

enum { PERSIST_HANDLE_INVALID = 0 }; 


typedef int (*persist_filler_t) (const char * entry, const zoidfs_handle_t *
      handle, void * fillerdata);

typedef struct
{
   /* buf should be at least PERSIST_HANDLE_MAXNAME;
    * return number of characters (excluding terminating zero)
    * written, 0 if not found */
   int (*persist_handle_to_filename) (void * data, const zoidfs_handle_t * handle, char * buf, unsigned
         int bufsize);

   /* lookup handle for filename. Return PERSIST_HANDLE_INVALID if not found
    * unless autoadd is set in which case a new handle is assigned */
   int (*persist_filename_to_handle) (void * data, const char * filename, zoidfs_handle_t * handle,
         int autoadd);  

   /* Remove mapping for the file; if prefix is nonzero, remove all files
    * starting with that prefix (e.g. directory removal)  */
   int (*persist_purge) (void * data, const char * filename, int prefix); 
   

   /* Rename file */
   int (*persist_rename) (void * data, const char * file1, const char *
         file2);
   
   /* Return all DB entries in directory dir (and call filler to store them)
    * Returns 1 if all done, 0 if filler aborted */ 
   int (*persist_readdir) (void * data, const char * dir, persist_filler_t
         filler, void * fillerdata); 

   /*  =============== private members ================== */ 

   void (*persist_cleanup) (void * data); 

   void * (*persist_init) (const char * inistr); 



   void * data; 

} persist_op_t; 

typedef struct
{
    void  (*initcon) (persist_op_t * op); 
    const char * name; 
} persist_module_t; 


static inline int persist_rename (persist_op_t * con, const char * f1, const
      char * f2)
{
   return con->persist_rename (con->data, f1, f2); 
}

static inline int persist_readdir (persist_op_t * con, const char * dir, persist_filler_t filler, 
      void * fillerdata)
{
   return con->persist_readdir (con->data, dir, filler, fillerdata); 
}


static inline int persist_handle_to_filename (persist_op_t * con,
      const zoidfs_handle_t * handle,
      char * buf, unsigned int bufsize)
{
   /* TODO: add namecache here */ 
   return con->persist_handle_to_filename (con->data, handle, buf, bufsize); 
}

static inline int persist_filename_to_handle (persist_op_t * con, const
      char * filename, zoidfs_handle_t * handle, int autoadd)
{
   /* TODO: add namecache here */ 
   return con->persist_filename_to_handle (con->data, filename, handle, autoadd); 
}


static inline int persist_purge (persist_op_t * con, const char * filename,
      int prefix)
{
   return con->persist_purge (con->data, filename, prefix); 
}


/* Open connection */ 
persist_op_t *  persist_init (const char * initstr); 


/* free connection */ 
void persist_done (persist_op_t * con); 

#endif


