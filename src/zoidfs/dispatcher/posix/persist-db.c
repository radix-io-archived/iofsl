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
#include <assert.h>

#include "zoidfs-util.h"
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

static inline void persist_db_generate_handle (zoidfs_handle_t * handle)
{
   assert ((sizeof (zoidfs_handle_t) % 4) == 0); 
   const int loops = sizeof (zoidfs_handle_t) / 4;
   uint32_t * dst = (uint32_t*) handle; 
   int i; 

   for (i=0; i<loops; ++i)
      *dst++ = lrand48 () & 0xffffffff; 

   /* First 4 bytes are reserved: set to zero */
   *(uint32_t*) handle = 0; 
}

static inline int persist_db_error (int ret)
{
   if (!ret)
      return ret; 
   persist_debug ("persist_db_error: %s\n", db_strerror(ret)); 
   exit (1); 
}

static int persist_db_purge (void * odata, 
      const char * filename, int prefix)
{
   persist_db_data_t * d = DBD(odata); 
   DBT key, data; 
   DBC * cursor; 
   int ret; 
   zoidfs_handle_t  handle; 

   memset (&key, 0, sizeof(key)); 

   key.data = (void*) filename; 
   key.size = key.ulen = strlen(filename); 
   key.flags = DB_DBT_USERMEM; 

   data.data = &handle; 
   data.ulen = sizeof(zoidfs_handle_t); 
   data.flags = DB_DBT_USERMEM; 

   persist_db_error(d->db->cursor (d->db, NULL, &cursor, 0)); 

   /* move cursor to record  */ 
   ret = cursor->c_get (cursor, &key, &data, DB_SET); 

   if (ret == DB_NOTFOUND)
   {
      persist_db_error(cursor->c_close (cursor)); 
      persist_debug ("persist_db_purge: %s not found\n", filename); 
      return 0; 
   }
   else
   {
      if (ret)
         persist_db_error (ret); 
   } 

   /* cursor now points to directory entry */
   /* remove entry */
   persist_db_error(cursor->c_del(cursor, 0)); 

   if (prefix)
   {
      /* continue removing until we no longer point to entries
       * of that directory */
      assert (0); 
   }

   persist_db_error(cursor->c_close (cursor)); 
   return 1; 

}

static int persist_db_handle_to_filename (void * odata, 
      const zoidfs_handle_t * handle, char * buf, unsigned int bufsize)
{
   persist_db_data_t * d = DBD(odata); 
   int ret;
   DBT key, data, pkey; 
   zoidfs_handle_t dummy; 
   
   persist_debug ("Looking up name %s\n", 
          zoidfs_handle_string (handle)); 

   memset (&key, 0, sizeof (key)); 
   memset (&data, 0, sizeof(data)); 

   key.data = (void*)handle; 
   key.ulen = key.size = sizeof(zoidfs_handle_t); 
   key.flags = DB_DBT_USERMEM; 

   pkey.data = buf; 
   pkey.ulen = bufsize; 
   pkey.flags = DB_DBT_USERMEM; 

   data.data = &dummy;
   data.ulen = sizeof(dummy); 
   data.flags = DB_DBT_USERMEM;
   
   ret = d->dbindex->pget (d->dbindex, NULL, &key, &pkey, &data, 0); 
   if (ret)
   {
      if (ret == DB_NOTFOUND)
      {
         persist_debug ("persist_db_handle_to_filename: invalid handle!\n"); 
         return 0; 
      }
      assert (ret != DB_BUFFER_SMALL); 
      persist_db_error (ret); 
   }
   assert (data.size < bufsize);

   /* the following needs to be true or our index is broken */ 
   assert(zoidfs_handle_equal(&dummy, handle)); 

   buf[pkey.size] = 0; 
   return pkey.size; 
}

static inline int persist_db_add_helper (persist_db_data_t * d,
      const char * filename,
      const zoidfs_handle_t * handle)
{
   DBT key, data; 
   memset (&key, 0, sizeof(key)); 
   memset (&data, 0, sizeof(data)); 

   data.data = (void*)handle; 
   data.ulen = data.size = sizeof(zoidfs_handle_t); 
   data.flags = DB_DBT_USERMEM; 

   key.data = (void*)filename; 
   key.ulen = key.size = strlen(filename); 
   key.flags = DB_DBT_USERMEM; 

   persist_debug ("Adding name %s to %s\n", 
         (const char*) key.data, zoidfs_handle_string (handle)); 
   return d->db->put (d->db, NULL, &key, &data, 
         DB_NOOVERWRITE);
}


static int persist_db_add  (void * odata, const char * buf, 
      const zoidfs_handle_t * handle)
{
   persist_db_data_t * d = DBD(odata); 
   zoidfs_handle_t newhandle = *handle; 

   /* clear reserved bytes */ 
   *(uint32_t*) &newhandle = 0; 

   int ret = persist_db_add_helper (d, buf, handle); 
   if (ret == 0)
      return 1; 

   if (ret == DB_KEYEXIST)
      return 0; 
         
   persist_db_error (ret); 
   return 0; 
}


static int persist_db_filename_to_handle (void * odata, 
      const char * filename, zoidfs_handle_t * handle, int autoadd)
{
   persist_db_data_t * d = DBD(odata); 
   int ret;
   DBT key, data; 

   memset (&key, 0, sizeof (key)); 
   memset (&data, 0, sizeof(data)); 

   do
   {
      key.data = (void*)filename; 
      key.ulen = key.size = strlen(filename); 
      key.flags = DB_DBT_USERMEM; 

      data.data = handle; 
      data.ulen = data.size = sizeof(zoidfs_handle_t); 
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
            return 0; 
         }
      }
      else
      {
         return 1; 
      }

      /* prepare to autoadd */
      persist_db_generate_handle (handle); 

      ret =  persist_db_add_helper (d , filename, handle); 

      if (ret != DB_KEYEXIST)
         persist_db_error (ret); 

      if (ret == DB_KEYEXIST)
      {
         /* somebody else added the file in the mean time:
            retry the whole thing */ 
         persist_debug ("Key collision\n"); 
         continue; 
      }

      /* add worked */ 
      return 1; 
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
   /*persist_db_data_t * d = DBD(odata); 
   DBC * cursor; 
   DBT key, data; 
  */
   return 0; 
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

   persist_db_data_t * data = calloc (sizeof(persist_db_data_t), 1);

   persist_db_error(db_create (&data->db, NULL, 0)); 
   persist_db_error(db_create (&data->dbindex, NULL, 0)); 

   persist_db_error(data->db->open (data->db, NULL, filename, NULL, DB_BTREE,
            DB_CREATE,0)); 
   
   /* hard links prevent the mapping of handle to filename being unique*/ 
   persist_db_error(data->dbindex->set_flags(data->dbindex, DB_DUP)); 

   if (filename)
      snprintf (buf, sizeof(buf)-1, "%s.idx", filename); 

   persist_db_error(data->dbindex->open (data->dbindex, NULL, 
            (filename ? buf : NULL ), NULL, DB_HASH, DB_CREATE,0)); 


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


static int persist_db_rename (void * odata, const char * f1, 
      const char * f2, int dir)
{
   assert (0); 
   return 0; 
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
   op->persist_add = persist_db_add; 
   op->persist_rename = persist_db_rename; 
}

persist_module_t persist_db_module = {
   .name = "db://",
   .initcon = persist_db_init_con
}; 

persist_module_t persist_mem_module = {
   .name = "memdb://",
   .initcon = persist_mem_init_con
}; 
