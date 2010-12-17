#include <zoidfs/hints/zoidfs-hints.h>

#include "zoidfs/zoidfs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "c-util/tools.h"
#include "c-util/quicklist.h"

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
    hint->list = (struct qlist_head * ) malloc(sizeof(struct qlist_head));

    /* initialize the hint list */
    hint->list->next = hint->list;
    hint->list->prev = hint->list;

    hint->copy = 0;

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

    /* store in hint list */
    qlist_add_tail(&(i->list), hint.list);

out:
    return ret;
}

int zoidfs_hint_delete(zoidfs_op_hint_t hint, char * key)
{
    int ret = 0;
    zoidfs_op_hint_item_t * item = NULL, * witem = NULL;

    /* look for the item */
    qlist_for_each_entry_safe(item, witem, hint.list, list)
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

            /* all done */
            break;
        }
    }

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
    qlist_for_each_entry_safe(item, witem, hint.list, list)
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
    qlist_for_each_entry_safe(item, witem, hint.list, list)
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
    zoidfs_op_hint_item_t * item = NULL, * witem = NULL;
   
    if(!nkeys)
    {
        ret = -1;
        goto out;
    }
 
    /* init the flag to 0 */
    *nkeys = 0;

    /* look for the item */
    qlist_for_each_entry_safe(item, witem, hint.list, list)
    {
        (*nkeys)++;
    }

out:
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
    qlist_for_each_entry_safe(item, witem, hint.list, list)
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
    qlist_for_each_entry_safe(item, witem, hint.list, list)
    {
        if(n == index)
        {
            /* get the key length */
            *keylen = strlen(item->key);

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
    newhint->list = oldhint->list;
    newhint->copy = 1;

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
    qlist_for_each_entry_safe(item, witem, oldhint.list, list)
    {
        /* setup the new hint */
        zoidfs_op_hint_item_t * nitem = malloc(sizeof(zoidfs_op_hint_item_t));
        nitem->key = strdup(item->key);
        memcpy(nitem->value, item->value, item->valuelen);
        nitem->valuelen = item->valuelen;

        /* add item to the new hint */
        qlist_add_tail(&(nitem->list), newhint->list);
    }
    
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
        qlist_for_each_entry_safe(item, witem, hint->list, list)
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
        free(hint->list);
    }

out:
    return ret;
}
