/*
 * TODO:
 *   * thread safety
 *   * make big/little endian safe
 */
#include <stdio.h>
#include <stdlib.h>
#include <db.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "persist.h"

/* size of bulk data access buffer */ 
#define BULK_SIZE 1*1024*1024

/*=========================================================================*/

typedef struct
{
   DB * db;
   DB * dbindex; 
} persist_db_data_t; 

/*=========================================================================*/

#define DBD(a) ((persist_db_data_t*)(a))

static inline uint64_t persist_db_generate_handle ()
{
   return (lrand48() & 0xffffffff) | ((lrand48() & 0xffffffff) << 32);
}

static inline int persist_db_error (int ret)
{
   if (!ret)
      return ret; 
   fprintf (stderr, "persist_db_error: %s\n", db_strerror(ret)); 
   exit (1); 
}

static void persist_db_purge (void * odata, 
      const char * filename, int prefix)
{

}

static int persist_db_handle_to_filename (void * odata, 
      persist_handle_t handle, char * buf, unsigned int bufsize)
{
   persist_db_data_t * d = DBD(odata); 
   int ret;
   DBT key, data; 

   memset (&key, 0, sizeof (key)); 
   memset (&data, 0, sizeof(data)); 

   key.data = &handle; 
   key.ulen = key.size = sizeof(handle); 
   key.flags = DB_DBT_USERMEM; 

   data.data = buf; 
   data.ulen = bufsize; 
   data.flags = DB_DBT_USERMEM; 

   ret = d->dbindex->get (d->dbindex, NULL, &key, &data, 0); 
   if (ret)
   {
      if (ret == DB_NOTFOUND)
      {
         fprintf (stderr, "persist_db_handle_to_filename: invalid handle!\n"); 
         return 0; 
      }
      assert (ret != DB_BUFFER_SMALL); 
      persist_db_error (ret); 
   }
   assert (data.size < bufsize);
   buf[data.size] = 0; 
   return data.size; 
}

static persist_handle_t persist_db_filename_to_handle (void * odata, 
      const char * filename, int autoadd)
{
   persist_db_data_t * d = DBD(odata); 
   int ret;
   DBT key, data; 
   persist_handle_t handle = PERSIST_HANDLE_INVALID; 

   memset (&key, 0, sizeof (key)); 
   memset (&data, 0, sizeof(data)); 

   do
   {
      key.data = &filename; 
      key.ulen = key.size = strlen(filename); 
      key.flags = DB_DBT_USERMEM; 

      data.data = &handle; 
      data.ulen = sizeof(handle); 
      data.flags = DB_DBT_USERMEM; 

      ret = d->db->get (d->db, NULL, &key, &data, 0); 
      if (ret)
      {
         if (ret != DB_NOTFOUND)
         {
            /* should not happen, or handle size changed or DB is corrupt */ 
            assert (ret != DB_BUFFER_SMALL); 
            persist_db_error (ret); 
         }
         /* no autoadd and not found, tell the user so */ 
         if (!autoadd)
         {
            return PERSIST_HANDLE_INVALID; 
         }
      }
      else
      {
         assert (handle != PERSIST_HANDLE_INVALID); 
         return handle; 
      }

      /* prepare to autoadd */
      handle = persist_db_generate_handle (); 

      data.data = &handle; 
      data.ulen = sizeof(handle); 
      data.flags = DB_DBT_USERMEM; 

      key.data = &filename; 
      key.ulen = key.size = strlen(filename); 
      key.flags = DB_DBT_USERMEM; 

      ret = d->db->put (d->db, NULL, &key, &data, 
            DB_NOOVERWRITE);

      if (ret != DB_KEYEXIST)
         persist_db_error (ret); 

      if (ret == DB_KEYEXIST)
      {
         /* somebody else added the file in the mean time:
            retry the whole thing */ 
         continue; 
      }

      /* add worked */ 
      return handle; 
   } while (1); 
}


static void persist_db_cleanup (void * data)
{
   persist_db_error(DBD(data)->db->close(DBD(data)->db, 0)); 
   persist_db_error(DBD(data)->dbindex->close(DBD(data)->dbindex, 0)); 
}

static void persist_db_seed_random ()
{
   int des = open ("/dev/random", O_RDONLY); 
   unsigned long seed; 
   read (des, &seed, sizeof (seed)); 
   close (des); 
   srand48 (seed); 
}

static int persist_db_readdir (void * odata, const char * dir, 
      persist_filler_t filler, void * fillerdata)
{
   persist_db_data_t * d = DBD(odata); 
   DBC * cursor; 
   DBT key, data; 

}

/* return the secundairy key given the data for the primary key */
static int persist_db_getindexkey (DB *secondary,
    const DBT *key, const DBT *data, DBT *result)
{
   *result = *data; 
   return 0; 
}

static void * persist_db_mem_init (const char * initstr, int persist)
{
   const char * filename = (persist ? initstr : 0);
   char buf[255]; 
   int ret;

   persist_db_data_t * data = calloc (sizeof(persist_db_data_t), 1);

   persist_db_error(db_create (&data->db, NULL, 0)); 
   persist_db_error(db_create (&data->dbindex, NULL, 0)); 

   persist_db_error(data->db->open (data->db, NULL, filename, NULL, DB_BTREE,
            DB_CREATE,0)); 

   if (filename)
      snprintf (buf, sizeof(buf)-1, "%s.idx", filename); 

   persist_db_error(data->dbindex->open (data->dbindex, NULL, 
            (filename ? buf : NULL, NULL), NULL, DB_HASH, DB_CREATE,0)); 


   /* associate index */
   persist_db_error(data->db->associate(data->db, NULL, 
            data->dbindex, persist_db_getindexkey, DB_IMMUTABLE_KEY));

   /* seed random generator */ 
   persist_db_seed_random (); 

   return data; 
}

static void * persist_db_init (const char * initstr)
{
   return persist_db_mem_init (initstr, 1); 
}

static void persist_db_init_con (persist_op_t * op)
{
   op->persist_handle_to_filename = persist_db_handle_to_filename; 
   op->persist_filename_to_handle = persist_db_filename_to_handle; 
   op->persist_purge = persist_db_purge; 
   op->persist_readdir = persist_db_readdir;
   op->persist_cleanup = persist_db_cleanup; 
   op->persist_init = persist_db_init; 
}


static void * persist_mem_init (const char * initstr)
{
   return persist_db_mem_init (initstr, 0); 
}

static void persist_mem_init_con (persist_op_t * op)
{
   op->persist_handle_to_filename = persist_db_handle_to_filename; 
   op->persist_filename_to_handle = persist_db_filename_to_handle; 
   op->persist_purge = persist_db_purge; 
   op->persist_readdir = persist_db_readdir;
   op->persist_cleanup = persist_db_cleanup; 
   op->persist_init = persist_mem_init; 
}

persist_module_t persist_db_module = {
   .name = "db://",
   .initcon = persist_db_init_con
}; 

persist_module_t persist_mem_module = {
   .name = "memdb://",
   .initcon = persist_mem_init_con
}; 
