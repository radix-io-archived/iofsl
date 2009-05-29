#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "persist.h"

#include "iofwd_config.h"
#include "env-parse.h"

#ifdef HAVE_DB
#include "persist-db.h"
#endif

#ifdef HAVE_MYSQL
#include "persist-mysql.h"
#endif

pthread_mutex_t persist_mutex = PTHREAD_MUTEX_INITIALIZER; 
int persist_do_debug = 0; 

static const persist_module_t * persist_modules[] = { 
#ifdef HAVE_MYSQL
   &persist_mysql_module, 
#endif
#ifdef HAVE_DB
   &persist_db_module, 
   &persist_mem_module,
#endif
   0
}; 

persist_op_t * persist_init (const char * initstr)
{
   static pthread_mutex_t mutex; 
   persist_op_t * ret = 0;

   pthread_mutex_lock (&mutex); 

   do
   {
      const persist_module_t ** module = persist_modules; 
      const int initlen = (initstr ? strlen (initstr) : 0); 

      ret = calloc (sizeof (persist_op_t), 1);

#ifndef NDEBUG
      if (env_parse_have_debug("persist"))
         persist_do_debug=1; 
#endif

      if (!initstr)
         break; 

      while (*module)
      {
         const int len = strlen ((*module)->name); 
         if ((initlen > len) && (strncmp ((*module)->name, initstr, len)==0))
            break; 
         ++module; 
      }

      if (!*module)
      {
         /* no module found */ 
         fprintf (stderr, "persist: no module found to handle %s\n", initstr); 
         exit (1); 
      }

      (*module)->initcon (ret); 

      ret->data = ret->persist_init (initstr + strlen((*module)->name)); 

      pthread_mutex_init (&ret->mutex, NULL); 

   } while (0); 

   pthread_mutex_unlock (&mutex); 

   return ret; 
}

void persist_done (persist_op_t * con)
{
   pthread_mutex_lock (&con->mutex); 
   con->persist_cleanup (con->data); 
   pthread_mutex_unlock (&con->mutex); 
   pthread_mutex_destroy (&con->mutex);

   free (con); 
}

