#include <assert.h>
#include <stdlib.h>

#include "iofwdhash.h"
#include "iofwdbool.h"


typedef struct iofwdh_node iofwdh_node; 

struct iofwdh_node
{
   void * key;
   void * data; 
   iofwdh_node * next; 
};

struct iofwdhash
{
   iofwdh_comparefunc_t compare;
   iofwdh_hashfunc_t    hash;
   int size; 
   void * user; 
   iofwdh_node * * buckets; 
   int itemcount; 
}; 

iofwdh_t iofwdh_init (int size, iofwdh_comparefunc_t compare, 
      iofwdh_hashfunc_t hash, void * user)
{
   iofwdh_t n = calloc (1, sizeof (struct iofwdhash)); 
   n->size = size; 
   n->buckets = calloc (size, sizeof(iofwdh_node)); 
   n->compare = compare; 
   n->hash = hash; 
   n->itemcount = 0; 
   n->user = user; 
   return n; 
}

static void iofwdh_destroychain (iofwdh_node * n)
{
   while (n)
   {
      iofwdh_node * next = n->next;
      free (n); 
      n = next; 
   }
}

void iofwdh_destroy (iofwdh_t table)
{
   unsigned int i;
   for (i=0; i<table->size; ++i)
     iofwdh_destroychain (table->buckets[i]); 
   free (table->buckets); 
   free (table); 
}

iofwdbool_t iofwdh_lookup (iofwdh_t table, void * key, void ** data)
{
   int bucket = table->hash (table->user, table->size, key); 
   assert (bucket < table->size); 
   const iofwdh_node * n = table->buckets[bucket];
   while (n)
   {
      if (table->compare (table->user, key, n->key))
      {
         if (data)
            *data = n->data; 
         return TRUE; 
      }
      n = n->next; 
   }
   return FALSE; 
}

void iofwdh_add (iofwdh_t table, void * key, void * data)
{
   int bucket = table->hash (table->user, table->size, key); 
   assert (bucket < table->size); 
   iofwdh_node * n = calloc (1, sizeof(iofwdh_node)); 
   n->key = key; 
   n->data = data; 
   n->next = table->buckets[bucket]; 
   table->buckets[bucket] = n;
   ++table->itemcount; 
}

int iofwdh_itemcount (iofwdh_t table)
{
   return table->itemcount; 
}

iofwdbool_t iofwdh_remove (iofwdh_t table, void * key, void ** data)
{
   int bucket = table->hash (table->user, table->size, key); 
   assert (bucket < table->size); 
   iofwdh_node * n = table->buckets[bucket];
   iofwdh_node * prev = 0; 
   while (n)
   {
      if (table->compare (table->user, key, n->key))
      {
         if (data)
            *data = n->data; 

         if (prev)
            prev->next = n->next;
         else
            table->buckets[bucket]=n->next; 

         free (n); 

         --table->itemcount; 

         return TRUE; 
      }
      prev = n; 
      n = n->next; 
   }
   return FALSE; 
}

iofwdbool_t iofwdh_walktable (iofwdh_t table, iofwdh_walkfunc_t func,
      void * funcuser)
{
   int i; 
   for (i=0; i<table->size; ++i)
   {
      iofwdh_node * n = table->buckets[i];
      while (n)
      {
         if (!func (table->user, funcuser, n->key, n->data))
            return FALSE; 
         n=n->next; 
      }
   }
   return TRUE; 
}
