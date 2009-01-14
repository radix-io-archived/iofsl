#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "persist.h"

#include "iofwd_config.h"

#ifdef HAVE_DB
#include "persist-db.h"
#endif

#ifdef HAVE_MYSQL
#include "persist-mysql.h"
#endif

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

   persist_op_t * ret = calloc (sizeof (persist_op_t), 1);


   char * saveptr = 0; 
   const persist_module_t ** module = persist_modules; 

   const int initlen = (initstr ? strlen (initstr) : 0); 
   
   if (!initstr)
      return 0; 


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
   return ret; 
}

void persist_done (persist_op_t * con)
{
   con->persist_cleanup (con->data); 
   free (con); 
}
