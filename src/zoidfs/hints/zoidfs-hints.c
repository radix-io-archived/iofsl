#include <zoidfs/hints/zoidfs-hints.h>

#include "zoidfs/zoidfs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "c-util/tools.h"

/*
 * zoidfs_hint_init() - initialize the hint list to 'size' elements
 */
zoidfs_op_hint_t * zoidfs_hint_init(int size)
{
    zoidfs_op_hint_t * new_op_hint = NULL;
    zoidfs_op_hint_t * cur_op_hint = NULL;
    int i = 0;

    /* allocate a new hint with 'size' elements */
    for(i = 0 ; i < size ; i++)
    {
        /* if this is the first element, make it the head of the hint list */
        if(i == 0)
        {
            cur_op_hint = (zoidfs_op_hint_t *) malloc(sizeof(zoidfs_op_hint_t));
            new_op_hint = cur_op_hint;
        }
        else
        {
            cur_op_hint->next = (zoidfs_op_hint_t *) malloc(sizeof(zoidfs_op_hint_t));
            cur_op_hint = cur_op_hint->next;
        }

        /* intialize the hint to empty */
        cur_op_hint->key = NULL;
        cur_op_hint->value = NULL;
        cur_op_hint->value_len = 0;
        cur_op_hint->type = ZOIDFS_HINT_TYPE_CHAR;
        cur_op_hint->next = NULL;
        cur_op_hint->encode = NULL;
        cur_op_hint->decode = NULL;
    }

    return new_op_hint;
}

static int zoidfs_hint_update_hint(zoidfs_op_hint_t * cur_op_hint, char * key, char * value, int value_len, int flags)
{
    /* cleanup the old hint data if there was old hint data */
    if(cur_op_hint->value)
    {
        free(cur_op_hint->value);
    }
    if(cur_op_hint->key && (ZOIDFS_HINTS_REUSE_KEY & flags) == 0)
    {
        free(cur_op_hint->key);
    }

    /* if we need zero copy, just update the pointers */
    if(flags & ZOIDFS_HINTS_ZC)
    {
        /* if we are resuing the key, don't update the key val */
        if((flags & ZOIDFS_HINTS_REUSE_KEY) == 0)
        {
            cur_op_hint->key = key;
        }
        cur_op_hint->value_len = value_len;
        cur_op_hint->value = value;
        cur_op_hint->encode = NULL;
        cur_op_hint->decode = NULL;
        cur_op_hint->type = ZOIDFS_HINT_TYPE_CHAR;
    }
    /* otherwise, reallocate the buffers and copy the data */
    else
    {
        /* if we are resuing the key, don't update the key val */
        if((flags & ZOIDFS_HINTS_REUSE_KEY) == 0)
        {
            cur_op_hint->key = (char *) malloc(sizeof(char) * strlen(key) + 1);
            strcpy(cur_op_hint->key, key);
        }
        cur_op_hint->value = (char *)malloc(sizeof(char) * value_len);
        memcpy(cur_op_hint->value, value, value_len);
        cur_op_hint->value_len = value_len;
        cur_op_hint->encode = NULL;
        cur_op_hint->decode = NULL;
        cur_op_hint->type = ZOIDFS_HINT_TYPE_CHAR;
    }
    return 0;
}

static int zoidfs_hint_update_non_unique(zoidfs_op_hint_t * cur_op_hint, char * key, char * value, int value_len, int flags)
{
    /* key can not be NULL */
    if(!key)
    {
        return -1;
    }

    while(cur_op_hint)
    {
        /* if we found a match */
        if(cur_op_hint->key != NULL &&
            strcmp(key, cur_op_hint->key) == 0)
        {
            if(value_len == cur_op_hint->value_len &&
                memcmp(cur_op_hint->value, value, value_len) == 0)
            {
                if(key)
                    free(key);
                if(value)
                    free(value);
            }
            else
            {
                if(key)
                    free(key);
                zoidfs_hint_update_hint(cur_op_hint, cur_op_hint->key, value, value_len, flags | ZOIDFS_HINTS_REUSE_KEY);
            }
            return 1;
        }
        cur_op_hint = cur_op_hint->next;
    }

    /* key is unique */
    return 0;
}

/*
 * zoidfs_hint_add() - add a hint keyed with 'key'
 */
int zoidfs_hint_add(zoidfs_op_hint_t ** op_hint, char * key, char * value, int value_len, int flags)
{
    zoidfs_op_hint_t * new_op_hint = NULL;
    zoidfs_op_hint_t * cur_op_hint = NULL;
    if(!op_hint)
    {
        return -1;
    }
    cur_op_hint = *op_hint;

    /* check if the hint was initialized to empty */
    if(cur_op_hint)
    {
        if(cur_op_hint->key == NULL &&
            cur_op_hint->value == NULL)
        {
            /* don't rebuild the hint, just copy the data into it */
            zoidfs_hint_update_hint(cur_op_hint, key, value, value_len, flags);
            return 0;
        }
    }

    /* if the list is empty, add the new hint as the head of the hint list */
    if(!cur_op_hint)
    {
        *op_hint = new_op_hint;
    }
    /* else, there are items in the list... add it to the end of the list if there isn't an empty node */
    else
    {
        if(zoidfs_hint_update_non_unique(cur_op_hint, key, value, value_len, flags))
        {
            return 0;
        }

        /* find the last hint in the list */
        while(cur_op_hint)
        {
            if(cur_op_hint->key == NULL &&
                cur_op_hint->value == NULL)
            {
                /* don't rebuild the hint, just copy the data into it */
                zoidfs_hint_update_hint(cur_op_hint, key, value, value_len, flags);
                break;
            }
            else
            {
                /* skip all hints that are not the last hint in the linked list */
                if(cur_op_hint->next != NULL)
                {
                    cur_op_hint = cur_op_hint->next;
                }
                /* if this is the last hint, add the new hint */
                else
                {
                    /* check if the key alread exists... update the value if it does */
                    if(strcmp(cur_op_hint->key, key) == 0)
                    {
                        /* if the key, value pair already exists in the list, delete the new pair */
                        if(value_len == cur_op_hint->value_len &&
                            memcmp(cur_op_hint->value, value, value_len) == 0)
                        {
                            if(key)
                                free(key);
                            if(value)
                                free(value);
                        }
                        /* else, update the item in the list */
                        else
                        {
                            if(key)
                                free(key);
                            zoidfs_hint_update_hint(cur_op_hint, cur_op_hint->key, value, value_len, flags | ZOIDFS_HINTS_REUSE_KEY);
                        }
                        break;
                    }
                    /* else, make a new hint in the linked list */
                    else
                    {
                        /* allocate a new hint */
                        new_op_hint = (zoidfs_op_hint_t *) malloc(sizeof(zoidfs_op_hint_t));
                        new_op_hint->value = NULL;
                        new_op_hint->value_len = 0;
                        new_op_hint->key = NULL;
                        zoidfs_hint_update_hint(new_op_hint, key, value, value_len, flags);
                        new_op_hint->next = NULL;
                        new_op_hint->encode = NULL;
                        new_op_hint->decode = NULL;
                        cur_op_hint->next = new_op_hint;
                        cur_op_hint->type = ZOIDFS_HINT_TYPE_CHAR;

                        break;
                    }
                }
            }
        }
    }

    return 0;
}

/*
 * zoidfs_hint_remove() - remove a hint keyed with 'key'
 */
int zoidfs_hint_remove(zoidfs_op_hint_t ** op_hint, char * key, int flags)
{
    zoidfs_op_hint_t * cur_op_hint = NULL;
    zoidfs_op_hint_t * prev_op_hint = NULL;
    zoidfs_op_hint_t * rm_op_hint = NULL;

    if(!op_hint)
    {
        return -1;
    }
    cur_op_hint = *op_hint;

    /* search the list of hints for the key */
    while(cur_op_hint)
    {
        /* if the current hint is the one we are looking for */
        if(cur_op_hint->key != NULL && strcmp(cur_op_hint->key, key) == 0)
        {
            /* copy the hint and remove the hint from the list */
            rm_op_hint = cur_op_hint;

            /* if we are not keeping hint nodes, remove them from the list */
            if((flags & ZOIDFS_HINTS_REUSE_HINTS) == 0)
            {
                /* if this is not the first hint */
                if(prev_op_hint)
                {
                    prev_op_hint->next = cur_op_hint->next;
                }
                /* if this is the first hint */
                else
                {
                    *op_hint = cur_op_hint->next;
                }
            }
            break;
        }
        /* otherwise, keep looking through the list */
        else
        {
            /* store the cur hint as the prev hint and update the cur hint as the next hint */
            prev_op_hint = cur_op_hint;
            cur_op_hint = cur_op_hint->next;
        }
    }

    /* remove the hint if it was found */
    if(rm_op_hint)
    {
        if(rm_op_hint->value)
        {
            free(rm_op_hint->value);
            rm_op_hint->value_len = 0;
            rm_op_hint->value = NULL;
        }
        if(rm_op_hint->key)
        {
            free(rm_op_hint->key);
            rm_op_hint->key = NULL;
        }
        /* if we are not reusing the hints, free the mem */
        if((flags & ZOIDFS_HINTS_REUSE_HINTS) == 0)
        {
            free(rm_op_hint);
        }
        return 0;
    }

    /* we did not find the hint... return error */
    return -1;
}

/*
 * zoidfs_hint_get() - get a hint keyed with 'key'
 */
char * zoidfs_hint_get(zoidfs_op_hint_t ** op_hint, char * key)
{
    zoidfs_op_hint_t * cur_op_hint = NULL;
    if(!op_hint)
    {
        return NULL;
    }
    cur_op_hint = *op_hint;

    /* if the list is empty, retun NULL */
    if(!cur_op_hint)
    {
        return NULL;
    }

    /* search the list of hints for the key */
    while(cur_op_hint)
    {
        /* if the current hint is the one we are looking for */
        if(strcmp(cur_op_hint->key, key) == 0)
        {
            /* copy the hint and remove the hint from the list */
            return cur_op_hint->value;
        }
        /* otherwise, keep looking through the list */
        else
        {
            cur_op_hint = cur_op_hint->next;
        }
    }

    /* could not find the key, return NULL */
    return NULL;
}

zoidfs_op_hint_t * zoidfs_hint_pop(zoidfs_op_hint_t ** op_hint)
{
    zoidfs_op_hint_t * cur_op_hint = NULL;

    /* if valid op_hint */
    if(op_hint)
    {
        /* store the head of the list */
        cur_op_hint = *op_hint;

        /* if the head was valid */
        if(cur_op_hint)
        {
            *op_hint = cur_op_hint->next;
            cur_op_hint->next = NULL;
        }
    }

    return cur_op_hint;
}

/*
 * zoidfs_hint_destroy() - get a hint keyed with 'key'
 */
int zoidfs_hint_destroy(zoidfs_op_hint_t ** op_hint)
{
    zoidfs_op_hint_t * cur_op_hint = NULL;
    if(!op_hint)
    {
        return -1;
    }
    cur_op_hint = *op_hint;

    /* while there are hint in the list */
    while(cur_op_hint)
    {
        /* store and update the cur hint */
        zoidfs_op_hint_t * rm_op_hint = cur_op_hint;
        cur_op_hint = cur_op_hint->next;

        /* if the hint is not null, free it */
        if(rm_op_hint)
        {
            if(rm_op_hint->value)
            {
                free(rm_op_hint->value);
                rm_op_hint->value = NULL;
            }
            if(rm_op_hint->key)
            {
                free(rm_op_hint->key);
                rm_op_hint->key = NULL;
            }
            free(rm_op_hint);
        }
    }

    return 0;
}

/*
 * zoidfs_hint_print() - print all the keys and values for this hint list
 */
int zoidfs_hint_print(zoidfs_op_hint_t ** op_hint)
{
    zoidfs_op_hint_t * cur_op_hint = NULL;
    int counter = 0;

    if(!op_hint)
    {
        return -1;
    }
    cur_op_hint = *op_hint;

    /* print all hints in the list */
    fprintf(stderr, "hint list:\n");
    while(cur_op_hint)
    {
        if(cur_op_hint->value)
        {
            fprintf(stderr, "hint %i: key = %s, value = %s\n", counter, cur_op_hint->key, cur_op_hint->value);
        }
        cur_op_hint = cur_op_hint->next;
        counter++;
    }

    return 0;
}

/*
 * zoidfs_hint_num_elements() - get the size of the hint list
 */
int zoidfs_hint_num_elements(zoidfs_op_hint_t ** op_hint)
{
    int size = 0;
    zoidfs_op_hint_t * cur_op_hint = NULL;

    if(op_hint)
    {
        cur_op_hint = *op_hint;
    }
    else
    {
        return -1;
    }

    while(cur_op_hint)
    {
        if(cur_op_hint->key != NULL && cur_op_hint->value != NULL && cur_op_hint->value_len != 0)
        {
            size++;
        }
        cur_op_hint = cur_op_hint->next;
    }

    return size;
}

/*
 * zoidfs_hint_index() - get the element in the hint list at index
 */
zoidfs_op_hint_t * zoidfs_hint_index(zoidfs_op_hint_t ** op_hint, int index)
{
    int pos = 0;
    zoidfs_op_hint_t * cur_op_hint = NULL;

    if(op_hint)
    {
        cur_op_hint = *op_hint;
    }
    else
    {
        return NULL;
    }

    while(cur_op_hint)
    {
        if(pos == index &&
            (cur_op_hint->key != NULL && cur_op_hint->value != NULL && cur_op_hint->value_len != 0) )
        {
            return cur_op_hint;
        }
        else
        {
            if(cur_op_hint->key != NULL && cur_op_hint->value != NULL && cur_op_hint->value_len != 0)
            {
                pos++;
            }
        }
        cur_op_hint = cur_op_hint->next;
    }

    return NULL;
}

char * zoidfs_hint_make_key(int key_len)
{
    return (char *) malloc(sizeof(char) * key_len);
}

int zoidfs_hint_rm_key(char * key)
{
    if(key)
    {
        free(key);
        return 0;
    }
    return 1;
}

char * zoidfs_hint_make_value(int value_len)
{
    return (char *) malloc(sizeof(char) * value_len);
}

int zoidfs_hint_rm_value(char * value)
{
    if(value)
    {
        free(value);
        return 0;
    }
    return 1;
}

/* encode the int value into the hint */
void encode_int(char ** pptr, void * value)
{
    *(int *) *(pptr) = *((int *)value);
    *pptr += sizeof(int);
}

/* encode the double value into the hint */
void encode_double(char ** pptr, void * value)
{
    *(double *) *(pptr) = *((double *)value);
    *pptr += sizeof(double);
}

/* decode the int value stored in the hint */
void decode_int(char ** pptr, void * value)
{
    *((int *)value) = *(int *) *(pptr);
    *pptr += sizeof(int);
}

/* decode the double value stored in the hint */
void decode_double(char ** pptr, void * value)
{
    *((double *)value) = *(double *) *(pptr);
    *pptr += sizeof(double);
}

/* new hint functions... at the moment, they do nothing */
int zoidfs_op_hint_create(zoidfs_op_hint_t * UNUSED(hint))
{
    return 0;
}

int zoidfs_op_hint_set(zoidfs_op_hint_t UNUSED(hint), char * UNUSED(key), char * UNUSED(value))
{
    return 0;
}

int zoidfs_op_hint_delete(zoidfs_op_hint_t UNUSED(hint), char * UNUSED(key))
{
    return 0;
}

int zoidfs_op_hint_get(zoidfs_op_hint_t UNUSED(hint), char * UNUSED(key), int UNUSED(valuelen), char * UNUSED(value), int * UNUSED(flag))
{
    return 0;
}

int zoidfs_op_hint_get_valuelen(zoidfs_op_hint_t UNUSED(hint), char * UNUSED(key), int * UNUSED(valuelen), int * UNUSED(flag))
{
    return 0;
}

int zoidfs_op_hint_get_nkeys(zoidfs_op_hint_t UNUSED(hint), int * UNUSED(nkeys))
{
    return 0;
}

int zoidfs_op_hint_get_nthkey(zoidfs_op_hint_t UNUSED(hint), int UNUSED(n), char * UNUSED(key))
{
    return 0;
}

int zoidfs_op_hint_dup(zoidfs_op_hint_t UNUSED(oldhint), zoidfs_op_hint_t * UNUSED(newhint))
{
    return 0;
}

int zoidfs_op_hint_free(zoidfs_op_hint_t * UNUSED(hint))
{
    return 0;
}
