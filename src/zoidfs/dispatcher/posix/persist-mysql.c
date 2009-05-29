#include <stdio.h>
#include <stdlib.h>
#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>
#include <string.h>
#include "persist.h"

/*=========================*/
/*=========================*/
typedef struct
{
     MYSQL  * con; 
} persist_mysql_t; 

static int persist_mysql_readdir (void * data, const char * dir, persist_filler_t filler, 
      void * fillerdata)
{
   return -1; 
}

static int persist_mysql_rename (void * data, const char * file1, const char * file2, int dir)
{
   return -1; 
}

static int persist_mysql_add (void * data, const char * filename, 
      const zoidfs_handle_t * handle)
{
   persist_mysql_t * d = (persist_mysql_t*) data; 
   char enc_handle[(sizeof(zoidfs_handle_t)*2)+1];
   char enc_filename[ZOIDFS_PATH_MAX*2+1]; 
   const char * stm = "INSERT INTO filemap (handle,filename) VALUES('%s','%s')"; 
   char * query = malloc (ZOIDFS_PATH_MAX*2+1+strlen(stm)+sizeof(zoidfs_handle_t)*2+1); 
   char  encodedhandle[sizeof(zoidfs_handle_t)*2+1]; 
   char * encodedfilename = malloc (ZOIDFS_PATH_MAX*2+1); 
   int error; 

   mysql_real_escape_string (d->con,encodedfilename,filename,strlen(filename)); 
   mysql_real_escape_string (d->con,encodedhandle, (const char*)&handle, sizeof(zoidfs_handle_t));  
   sprintf (query, stm, encodedhandle, encodedfilename); 
   error = mysql_query(d->con, query); 
   free (query); 
   free (encodedfilename); 
}

static int persist_mysql_purge (void * data, const char * filename, int prefix)
{
   persist_mysql_t * d = (persist_mysql_t *)data; 
   /* note: not optimized */
   const char * stm1 = "DELETE FROM filemap WHERE name like '%s'";
   const char * stm2 = "DELETE FROM filemap WHERE name like '%s\%'";
   char * query = malloc (ZOIDFS_PATH_MAX*2+1+strlen(stm2)); 
   char * encoded = malloc (ZOIDFS_PATH_MAX*2+1);
   int error; 

   mysql_real_escape_string (d->con,encoded,filename,strlen(filename)); 
   sprintf (query, (prefix ? stm2 : stm1), encoded); 
   error = mysql_query(d->con, query); 
   free (query); 
   free (encoded); 
   return 0; 
}

static int persist_mysql_handle_to_filename (void * data, 
      const zoidfs_handle_t* handle, char * buf, unsigned int bufsize)
{
   persist_mysql_t * d = (persist_mysql_t *) data; 
   char encodedhandle[sizeof(zoidfs_handle_t)*2+1]; 
   char query[512]; 
   MYSQL_RES * result;
   MYSQL_ROW * row; 

   mysql_real_escape_string (d->con,encodedhandle, &handle, sizeof(zoidfs_handle_t)); 

   sprintf(query, "SELECT filename FROM filemap WHERE handle='%s' LIMIT 1",
         encodedhandle);
   mysql_query(d->con,query);
   result = mysql_store_result (d->con); 
   while ((row = mysql_fetch_row (result)))
   {
      assert(strlen((const char*)row[0])<bufsize); 
      strcpy (buf,(const char*) row[0]); 
      return 1; 
   }
   /* TODO: check return value codes */
   return 0; 
}

static int persist_mysql_filename_to_handle (void * data, 
      const char * filename, zoidfs_handle_t * handle, int autoadd)
{
   persist_mysql_t * d = (persist_mysql_t *) data; 
   char * encoded = malloc (ZOIDFS_PATH_MAX*2+1); 
   char * query = malloc (ZOIDFS_PATH_MAX*2+256); 
   MYSQL_RES * result;
   MYSQL_ROW  row; 

   mysql_real_escape_string (d->con,encoded, &handle, sizeof(zoidfs_handle_t)); 

   sprintf(query, "SELECT handle FROM filemap WHERE filename='%s' LIMIT 1",
         encoded);
   mysql_query(d->con,query);
   free (encoded); 
   free (query); 
   result = mysql_store_result (d->con); 
   while ((row = mysql_fetch_row (result)))
   {
      memcpy (handle, row[0], sizeof(zoidfs_handle_t)); 
      return 1; 
   }
   /* TODO: check return value codes */
   return 0; 
}

static void persist_mysql_error (MYSQL *conn, char *message)
{
  fprintf (stderr, "%s\n", message);
  if (conn != NULL)
  {
    fprintf (stderr, "Error %u (%s): %s\n",
             mysql_errno (conn), mysql_sqlstate (conn), mysql_error (conn));
  }
}
   

static void * persist_mysql_init (const char * initstr)
{
   const char * hostname = "localhost"; 
   unsigned int portnumber = 3306; 
   const char * username = "iofwd"; 
   const char * password = "IOFWDPass"; 
   const char * database = "iofwd"; 
   /* parse initstr */ 

   persist_mysql_t * data = calloc (sizeof(persist_mysql_t),1); 


   if (mysql_library_init (0, NULL, NULL))
   {
      fprintf (stderr, "Failed to initialize mysql library!\n"); 
      exit (1); 
   }
   data->con = mysql_init (NULL);
   if (data->con == NULL)
   {
      fprintf (stderr, "Failed to initialize mysql!\n"); 
      exit (1); 
   }

   if (!mysql_real_connect(data->con, hostname, username,
            password, database, portnumber, NULL, 0))
   {
      fprintf (stderr, "Failed to connect to DB: %s\n", 
            mysql_error(data->con)); 
      mysql_close (data->con); 
      exit (1); 
   }

}

static void persist_mysql_cleanup (void * data)
{
   persist_mysql_t * d = (persist_mysql_t*) data; 
   mysql_close (d->con); 
   mysql_library_end(); 
}

static void persist_mysql_init_con (persist_op_t * op)
{
   op->persist_handle_to_filename = persist_mysql_handle_to_filename;
   op->persist_filename_to_handle = persist_mysql_filename_to_handle;
   op->persist_purge = persist_mysql_purge;
   op->persist_readdir = persist_mysql_readdir;
   op->persist_cleanup = persist_mysql_cleanup;
   op->persist_init = persist_mysql_init; 
}

persist_module_t persist_mysql_module = { 
   .name = "mysql://", 
   .initcon = persist_mysql_init_con }; 

