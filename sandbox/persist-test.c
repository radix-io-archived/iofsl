#include <stdio.h>
#include <stdlib.h>

/* NOTE: probably has some memory leaks */

#include "iofwd_config.h"
#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif
#include "persist.h"
#include "persist-lexer.h"
#include "zoidfs-util.h"

#define MAXARGS 128
 	
static char *line_read = (char *)NULL;
static int quit = 0; 
static persist_op_t * op; 

typedef int (*commandfunc_t) (); 

static const char * active_command = 0; 
static int          param_count = 0;     
static const char *  args[MAXARGS]; 
static commandfunc_t command; 


typedef struct
{
   const char * name; 
   commandfunc_t func; 
} command_t; 

static int command_quit (); 
static int command_lookup (); 
static int command_map (); 
static int command_help (); 
static int command_purge (); 

command_t defined_commands[] = 
{ { "quit", command_quit },
  { "lookup", command_lookup }, 
  { "map", command_map },
  { "help", command_help },
  { "purge", command_purge }
}; 


#ifdef HAVE_LIBREADLINE
/* Read a string, and return a pointer to it.
   Returns NULL on EOF. */
static char * rl_gets ()
{
  /* If the buffer has already been allocated,
     return the memory to the free pool. */
  if (line_read)
    {
      free (line_read);
      line_read = (char *)NULL;
    }

  /* Get a line from the user. */
  line_read = readline ("Persist> ");

  /* If the line has any text in it,
     save it on the history. */
  if (line_read && *line_read)
    add_history (line_read);

  return (line_read);
}
#else
static char * rl_gets ()
{
   static char buf[255]; 
   return fgets(buf, sizeof(buf)-1,stdin); 
}
#endif

static int command_purge ()
{
   int i; 
   for (i=0; i<param_count; ++i)
   {
      printf ("Purging %s\n", args[i]); 
      persist_purge (op, args[i], 0); 
   }
   return 1; 
}

static int command_help ()
{
   printf ("commands:\n");
   printf ("   lookup filename...        Lookup handle for file\n"); 
   printf ("   map    handle...          Lookup filename for handle\n"); 
   printf ("   quit                      Exit test\n"); 
   printf ("Note: filenames need to start with /\n"); 
   return 1; 
}

static int command_quit ()
{
   quit = 1; 
   return 1; 
}

static int command_lookup ()
{
   int i;
   for (i=0; i<param_count; ++i)
   {
      char buf[255]; 

      zoidfs_handle_t handle; 
      persist_filename_to_handle (op, args[i], &handle,  1); 
      zoidfs_handle_to_text (&handle, buf, sizeof(buf)); 
      printf ("%s --> %s\n", args[i], buf); 
   }
   return 1; 
}

static int command_map ()
{
   int i; 
   for (i=0; i<param_count; ++i)
   {
      int ret; 
      zoidfs_handle_t handle; 
      char buf[PERSIST_HANDLE_MAXNAME]; 
      
      if (!zoidfs_text_to_handle (args[i], &handle))
      {
         printf ("Could not parse handle: %s\n", args[i]); 
         return 1; 
      }

      ret = persist_handle_to_filename (op, &handle, buf, sizeof(buf)); 
      if (!ret)
      {
         printf ("Error mapping %s!\n", args[i]); 
      }
      else
      {
         printf ("%s -> %s\n", args[i], buf); 
      }
   }
   return 1; 
} 

// called by the parser
int start_command (const char * name)
{
   const int size = sizeof(defined_commands)/sizeof(defined_commands[0]);
   int i; 

   active_command = name; 
   param_count = 0; 

   command = 0; 
   for (i=0; i<size; ++i)
   {
      if (!strcasecmp (name, defined_commands[i].name))
      {
         command = defined_commands[i].func; 
         break; 
      }
   }
   if (!command)
   {
      fprintf (stderr, "Unknown command: %s!\n", name); 
      free ((void*)active_command); 
      return 0; 
   }
   return 1; 
}

int param_command (const char * param)
{
   args[param_count++] = param; 
   return 1; 
}

int end_command ()
{
   int i; 
   /* validate number of arguments */

   command (); 

   free ((void*)active_command); 
   for (i=0; i<param_count; ++i)
      free ((void*) args[i]); 
   return 1; 
}

int execute_command (const char * str)
{
   YY_BUFFER_STATE buf = yy_scan_string (str);
   yy_switch_to_buffer (buf); 

   yyparse (); 
   yy_delete_buffer (buf); 
   return 1; 
}

int main (int argc, char ** args)
{
   const char * line;

   if (argc != 2)
   {
      fprintf (stderr, "need persistdb string!\n"); 
      exit (1); 
   }

   op = persist_init (args[1]); 
   if (!op)
   {
      fprintf (stderr, "Error initializing persist layer!"); 
      exit(1); 
   }

   while ((!quit) && (line = rl_gets ()))
    {
      execute_command (line); 
   }

   persist_done (op); 
   return EXIT_SUCCESS; 
}
