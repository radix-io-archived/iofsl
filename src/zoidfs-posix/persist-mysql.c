#include <stdio.h>
#include <stdlib.h>
#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>
#include "persist.h"

/*=========================*/
static MYSQL  * persist_mysql; 
/*=========================*/

persist_module_t persist_mysql_module = { 
   .name = "mysql://" }; 

static void persist_mysql_purge (const char * filename)
{
}

static int persist_mysql_handle_to_filename (persist_handle_t handle, char * buf, unsigned int bufsize)
{
}

static persist_handle_t persist_mysql_filename_to_handle (const char * filename)
{
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

static void persist_mysql_init (const char * initstr)
{
   const char * hostname = "localhost"; 
   unsigned int portnumber = 3306; 
   const char * username = "iofwd"; 
   const char * password = "IOFWDPass"; 
   const char * database = "iofwd"; 
   /* parse initstr */ 

   if (mysql_library_init (0, NULL, NULL))
   {
      fprintf (stderr, "Failed to initialize mysql library!\n"); 
      exit (1); 
   }
   persist_mysql = mysql_init (NULL);
   if (persist_mysql == NULL)
   {
      fprintf (stderr, "Failed to initialize mysql!\n"); 
      exit (1); 
   }

   if (!mysql_real_connect(persist_mysql, hostname, username,
            password, database, portnumber, NULL, 0))
   {
      fprintf (stderr, "Failed to connect to DB: %s\n", 
            mysql_error(persist_mysql)); 
      mysql_close (persist_mysql); 
      exit (1); 
   }
}

static void persist_mysql_done ()
{
   mysql_close (persist_mysql); 
   mysql_library_end(); 
}
