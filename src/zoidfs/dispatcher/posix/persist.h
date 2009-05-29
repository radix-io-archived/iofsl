#ifndef IOFWD_POSIX_PERSIST_H
#define IOFWD_POSIX_PERSIST_H

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "zoidfs.h"

extern pthread_mutex_t persist_mutex; 

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
   
   /* Add mapping: fails if it exists 
    * (used when a file /must/ have a certain handle (e.g. hardlinks)
    */
   int (*persist_add) (void * data, const char * filename, const
            zoidfs_handle_t * handle); 

   /* Rename file */
   int (*persist_rename) (void * data, const char * file1, const char *
         file2, int dir);
   
   /* Return all DB entries in directory dir (and call filler to store them)
    * Returns 1 if all done, 0 if filler aborted */ 
   int (*persist_readdir) (void * data, const char * dir, persist_filler_t
         filler, void * fillerdata); 

   /*  =============== private members ================== */ 

   void (*persist_cleanup) (void * data); 

   void * (*persist_init) (const char * inistr); 



   void * data; 

   pthread_mutex_t mutex; 

} persist_op_t; 

typedef struct
{
    void  (*initcon) (persist_op_t * op); 
    const char * name; 
} persist_module_t; 


static inline int persist_add (persist_op_t * con, const char * name,
      const zoidfs_handle_t * handle)
{
   int ret;
   pthread_mutex_lock (&con->mutex); 
   ret= con->persist_add (con->data, name, handle); 
   pthread_mutex_unlock (&con->mutex); 
   return ret; 
}

static inline int persist_rename (persist_op_t * con, const char * f1, const
      char * f2, int dir)
{
   int ret;
   pthread_mutex_lock (&con->mutex); 
   ret= con->persist_rename (con->data, f1, f2, dir); 
   pthread_mutex_unlock (&con->mutex); 
   return ret; 
}

static inline int persist_readdir (persist_op_t * con, const char * dir, persist_filler_t filler, 
      void * fillerdata)
{
   int ret;
   pthread_mutex_lock (&con->mutex); 
   ret= con->persist_readdir (con->data, dir, filler, fillerdata); 
   pthread_mutex_unlock (&con->mutex); 
   return ret; 
}


static inline int persist_handle_to_filename (persist_op_t * con,
      const zoidfs_handle_t * handle,
      char * buf, unsigned int bufsize)
{
   /* TODO: add namecache here */ 
   int ret;
   pthread_mutex_lock (&con->mutex); 
   ret= con->persist_handle_to_filename (con->data, handle, buf, bufsize); 
   pthread_mutex_unlock (&con->mutex); 
   return ret; 
}

static inline int persist_filename_to_handle (persist_op_t * con, const
      char * filename, zoidfs_handle_t * handle, int autoadd)
{
   /* TODO: add namecache here */ 
   int ret;
   pthread_mutex_lock (&con->mutex); 
   ret= con->persist_filename_to_handle (con->data, filename, handle, autoadd); 
   pthread_mutex_unlock (&con->mutex); 
   return ret; 
}


static inline int persist_purge (persist_op_t * con, const char * filename,
      int prefix)
{
   int ret;
   pthread_mutex_lock (&con->mutex); 
   ret= con->persist_purge (con->data, filename, prefix); 
   pthread_mutex_unlock (&con->mutex); 
   return ret; 
}


/* Open connection */ 
persist_op_t *  persist_init (const char * initstr); 


/* free connection */ 
void persist_done (persist_op_t * con); 


#ifndef NDEBUG
extern int persist_do_debug; 
#define persist_debug(format, ...) if(persist_do_debug)\
               fprintf (stderr, "persist: debug: " format, ##__VA_ARGS__)
#else
#define persist_debug(format, ...) {}
#endif

#endif


