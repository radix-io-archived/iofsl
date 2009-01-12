#include <stdio.h>
#include <stdlib.h>
#include <db.h>
#include "persist.h"

static void persist_db_purge (void * data, 
      const char * filename)
{
}

static int persist_db_handle_to_filename (void * data, 
      persist_handle_t handle, char * buf, unsigned int bufsize)
{
}

static persist_handle_t persist_db_filename_to_handle (void * data, 
      const char * filename)
{
}


static void * persist_mysql_init (const char * initstr)
{
}

static void persist_db_cleanup (void * data)
{
}

static int persist_db_readdir (void * data, const char * dir, 
      persist_filler_t filler, void * fillerdata)
{
}

static void * persist_db_mem_init (const char * initstr, int persist)
{
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
   .name = "mem://",
   .initcon = persist_mem_init_con
}; 
