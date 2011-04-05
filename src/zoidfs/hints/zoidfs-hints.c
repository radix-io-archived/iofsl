#include <zoidfs/hints/zoidfs-hints.h>

#include "zoidfs/zoidfs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "c-util/tools.h"
#include "c-util/quicklist.h"

int zoidfs_hint_get_max_size()
{
    return ZOIDFS_HINT_MAX_HINTS * (ZOIDFS_HINT_MAX_KEY_SIZE + ZOIDFS_HINT_MAX_VALUE_SIZE);
}

int zoidfs_hint_create(zoidfs_op_hint_t * hint)
{
    int ret = 0;

    /* check for a valid hint object */
    if(!hint)
    {
        ret = -1;
        goto out;
    }

    /* allocate the list */
    hint->hlist = (struct qlist_head * ) malloc(sizeof(struct qlist_head));

    /* initialize the hint list */
    hint->hlist->next = hint->hlist;
    hint->hlist->prev = hint->hlist;

    hint->num_items = 0;
    hint->copy = 0;

out:
    return ret;
}

int zoidfs_hint_set_raw(zoidfs_op_hint_t hint, char * key, char * value, int valuelen)
{
    int ret = 0;
    if(!key)
    {
        ret = -1;
        goto out;
    }
    if(!value)
    {
        ret = -1;
        goto out;
    }
    if(hint.num_items >= ZOIDFS_HINT_MAX_HINTS)
    {
        ret = -1;
        goto out;
    }
    if(strlen(key) + 1 > ZOIDFS_HINT_MAX_KEY_SIZE)
    {
        ret = 1;
        goto out;
    }
    if(valuelen > ZOIDFS_HINT_MAX_KEY_SIZE)
    {
        ret = 1;
        goto out;
    }

    /* create the hint item */
    zoidfs_op_hint_item_t * i = (zoidfs_op_hint_item_t *)malloc(sizeof(zoidfs_op_hint_item_t));

    /* set the key and value */
    i->key = key;
    i->value = value;
    i->valuelen = valuelen;

    hint.num_items++;

    /* store in hint list */
    qlist_add_tail(&(i->list), hint.hlist);

out:
    return ret;
}

int zoidfs_hint_set(zoidfs_op_hint_t hint, char * key, char * value, int bin_length)
{
    int ret = 0;
    if(!key)
    {
        ret = -1;
        goto out;
    }
    if(!value)
    {
        ret = -1;
        goto out;
    }
    if(hint.num_items >= ZOIDFS_HINT_MAX_HINTS)
    {
        ret = -1;
        goto out;
    }
    if(strlen(key) + 1 > ZOIDFS_HINT_MAX_KEY_SIZE)
    {
        ret = 1;
        goto out;
    }
    if((strlen(value) > ZOIDFS_HINT_MAX_KEY_SIZE && bin_length == 0) || (bin_length > ZOIDFS_HINT_MAX_KEY_SIZE))
    {
        ret = 1;
        goto out;
    }

    /* create the hint item */
    zoidfs_op_hint_item_t * i = (zoidfs_op_hint_item_t *)malloc(sizeof(zoidfs_op_hint_item_t));

    /* if the value is binary data */
    if(bin_length)
    {
        i->key = strdup(key);
        i->value = malloc(bin_length);
        memcpy(i->value, value, bin_length);
        i->valuelen = bin_length;
    }
    /* else it is a string */
    else
    {
        /* set values */
        i->key = strdup(key);
        i->value = strdup(value);
        i->valuelen = strlen(value) + 1;
    }

    hint.num_items++;

    /* store in hint list */
    qlist_add_tail(&(i->list), hint.hlist);

out:
    return ret;
}

int zoidfs_hint_delete(zoidfs_op_hint_t hint, char * key)
{
    int ret = 0;
    zoidfs_op_hint_item_t * item = NULL, * witem = NULL;

    /* look for the item */
    qlist_for_each_entry_safe(item, witem, hint.hlist, list)
    {
        /* compare the key */
        if(strcmp(key, item->key) == 0)
        {
            /* delete the item from the list */
            qlist_del(&(item->list));

            /* free the item */
            free(item->key);
            free(item->value);
            item->valuelen = 0;
            free(item);
    
            hint.num_items--;

            /* all done */
            break;
        }
    }

    return ret;
}

int zoidfs_hint_delete_all(zoidfs_op_hint_t hint)
{
    int ret = 0;
    zoidfs_op_hint_item_t * item = NULL, * witem = NULL;

    /* look for the item */
    qlist_for_each_entry_safe(item, witem, hint.hlist, list)
    {
        /* delete the item from the list */
        qlist_del(&(item->list));

        /* free the item */
        free(item->key);
        free(item->value);
        item->valuelen = 0;
        free(item);
    
        hint.num_items--;
    }

    return ret;
}

void * zoidfs_hint_get_raw(zoidfs_op_hint_t hint, char * key, int valuelen, int * flag)
{
    void * ret = NULL;
    zoidfs_op_hint_item_t * item = NULL, * witem = NULL;

    if(!key)
    {
        ret = NULL;
        goto out;
    }
    if(!flag)
    {
        ret = NULL;
        goto out;
    }

    /* init the flag to 0 */
    *flag = 0;

    /* look for the item */
    qlist_for_each_entry_safe(item, witem, hint.hlist, list)
    {
        /* compare the key */
        if(strcmp(key, item->key) == 0)
        {
            /* mark the hint as found */
            *flag = zfsmin(item->valuelen, valuelen);

            /* copy the data */
            ret = item->value;

            /* all done */
            break;
        }
    }

out:
    return ret;
}

int zoidfs_hint_get(zoidfs_op_hint_t hint, char * key, int valuelen, char * value, int * flag)
{
    int ret = 0;
    zoidfs_op_hint_item_t * item = NULL, * witem = NULL;

    if(!key)
    {
        ret = -1;
        goto out;
    }
    if(!value)
    {
        ret = -1;
        goto out;
    }
    if(!flag)
    {
        ret = -1;
        goto out;
    }

    /* init the flag to 0 */
    *flag = 0;

    /* look for the item */
    qlist_for_each_entry_safe(item, witem, hint.hlist, list)
    {
        /* compare the key */
        if(strcmp(key, item->key) == 0)
        {
            /* mark the hint as found */
            *flag = zfsmin(item->valuelen, valuelen);

            /* copy the data */
            memcpy(value, item->value, zfsmin(item->valuelen, valuelen));
            /* all done */
            break;
        }
    }

out:
    return ret;
}

int zoidfs_hint_get_valuelen(zoidfs_op_hint_t hint, char * key, int * valuelen, int * flag)
{
    int ret = 0;
    zoidfs_op_hint_item_t * item = NULL, * witem = NULL;
    
    if(!key)
    {
        ret = -1;
        goto out;
    }
    if(!valuelen)
    {
        ret = -1;
        goto out;
    }
    if(!flag)
    {
        ret = -1;
        goto out;
    }

    /* init the flag to 0 */
    *flag = 0;

    /* look for the item */
    qlist_for_each_entry_safe(item, witem, hint.hlist, list)
    {
        /* compare the key */
        if(strcmp(key, item->key) == 0)
        {
            /* mark the hint as found */
            *flag = 1;

            /* get the value length */
            *valuelen = item->valuelen;

            /* all done */
            break;
        }
    }

out:
    return ret;
}

int zoidfs_hint_get_nkeys(zoidfs_op_hint_t hint, int * nkeys)
{
    int ret = 0;
   
    if(!nkeys)
    {
        ret = -1;
        goto out;
    }
 
    *nkeys = qlist_count(hint.hlist);

out:
    return ret;
}

void * zoidfs_hint_get_nthkey_raw(zoidfs_op_hint_t hint, int n)
{
    void * ret = NULL;
    int index = 0;
    zoidfs_op_hint_item_t * item = NULL, * witem = NULL;

    /* look for the item */
    qlist_for_each_entry_safe(item, witem, hint.hlist, list)
    {
        if(n == index)
        {
            /* copy the key */
            ret = item->key;

            /* all done */
            break;
        }
        index++;
    }

    return ret;
}

int zoidfs_hint_get_nthkey(zoidfs_op_hint_t hint, int n, char * key)
{
    int ret = 0;
    int index = 0;
    zoidfs_op_hint_item_t * item = NULL, * witem = NULL;

    if(!key)
    {
        ret = -1;
        goto out;
    }
    
    /* look for the item */
    qlist_for_each_entry_safe(item, witem, hint.hlist, list)
    {
        if(n == index)
        {
            /* copy the key */
            strcpy(key, item->key);

            /* all done */
            break;
        }
        index++;
    }
out:
    return ret;
}

int zoidfs_hint_get_nthkeylen(zoidfs_op_hint_t hint, int n, int * keylen)
{
    int ret = 0;
    int index = 0;
    zoidfs_op_hint_item_t * item = NULL, * witem = NULL;

    if(!keylen)
    {
        ret = -1;
        goto out;
    }
    
    /* look for the item */
    qlist_for_each_entry_safe(item, witem, hint.hlist, list)
    {
        if(n == index)
        {
            /* get the key length */
            *keylen = strlen(item->key) + 1;

            /* all done */
            break;
        }
        index++;
    }
out:
    return ret;
}

int zoidfs_hint_copy(zoidfs_op_hint_t * oldhint, zoidfs_op_hint_t * newhint)
{
    int ret = 0;
    if(!newhint)
    {
        ret = -1;
        goto out;
    }

    if(!oldhint)
    {
        ret = -1;
        goto out;
    }

    /* copy the list pointer */
    newhint->hlist = oldhint->hlist;
    newhint->copy = 1;
    newhint->num_items = oldhint->num_items;

out:
    return ret;
}

int zoidfs_hint_dup(zoidfs_op_hint_t oldhint, zoidfs_op_hint_t * newhint)
{
    int ret = 0;
    zoidfs_op_hint_item_t * item = NULL, * witem = NULL;

    if(!newhint)
    {
        ret = -1;
        goto out;
    }

    /* look for the item */
    qlist_for_each_entry_safe(item, witem, oldhint.hlist, list)
    {
        /* setup the new hint */
        zoidfs_op_hint_item_t * nitem = malloc(sizeof(zoidfs_op_hint_item_t));
        nitem->key = strdup(item->key);
        nitem->value = malloc(sizeof(char) * item->valuelen);
        memcpy(nitem->value, item->value, item->valuelen);
        nitem->valuelen = item->valuelen;

        /* add item to the new hint */
        qlist_add_tail(&(nitem->list), newhint->hlist);
    }
    
    newhint->num_items = oldhint.num_items;

out:
    return ret;
}

int zoidfs_hint_free(zoidfs_op_hint_t * hint)
{
    int ret = 0;
    zoidfs_op_hint_item_t * item = NULL, * witem = NULL;

    if(!hint)
    {
        ret = -1;
        goto out;
    }

    if(!hint->copy)
    {
        /* look for the item */
        qlist_for_each_entry_safe(item, witem, hint->hlist, list)
        {
            /* delete the item from the list */
            qlist_del(&(item->list));

            /* free the item */
            free(item->key);
            free(item->value);
            item->valuelen = 0;
            free(item);
        }

        /* dealloc the list */
        free(hint->hlist);
    }

    hint->num_items = 0;

out:
    return ret;
}
