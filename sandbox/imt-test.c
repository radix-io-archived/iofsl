#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "c-util/interval-merge-tree.h"

#define NUM_DEL_KEYS 4
#define KEYSPACE 4000
#define ISIZE 4

int get_time(struct timespec * timeval)
{
    clock_gettime( CLOCK_REALTIME, timeval );
    return 0;
}

double elapsed_time(struct timespec * t1, struct timespec * t2)
{
    return ((double) (t2->tv_sec - t1->tv_sec) +
        1.0e-9 * (double) (t2->tv_nsec - t1->tv_nsec) );
}

int main(int argc, char * argv[])
{
    interval_merge_tree_node_t * root = NULL;
    interval_merge_tree_key_t * keys = NULL;
    struct timespec t1, t2;
    int num_keys = atoi(argv[1]);
    size_t key_space = atoi(argv[2]);

    int i = 0;

    srand((0));

    /* create the keys */
    keys = (interval_merge_tree_key_t *)malloc(sizeof(interval_merge_tree_key_t) * num_keys);
    for(i = 0 ; i < num_keys ; i++)
    {
        size_t * v = (size_t *) malloc(sizeof(size_t));
        *v = (rand() % key_space) * ISIZE;
        keys[i].key = v;
        keys[i].value = v;
    }

    get_time(&t1);
    /* insert and merge the keys */
    for(i = 0 ; i < num_keys ; i++)
    {
        int ret = 0;
        //fprintf(stderr, "INSERT # %i: key = %i\n", i, *(size_t *)keys[i]->key);
        interval_merge_tree_node_t * nn = interval_merge_tree_create_node();
        interval_merge_tree_interval_ll_init(nn, (size_t *)keys[i].key);
        nn->interval.start = *(size_t *)keys[i].key;
        nn->interval.end = *(size_t *)keys[i].key + ISIZE;
        nn->size = 1;
        nn->max = nn->interval.end;
        ret = interval_merge_tree_merge_intervals(&root, &nn, keys[i]);
        if(ret)
        {
            fprintf(stderr, "fail cleanup\n");
            interval_merge_tree_interval_ll_destroy(nn->ll_head);
            nn->ll_head = NULL;
            nn->ll_tail = NULL;
            interval_merge_tree_destroy_node(nn);
        }
        //interval_merge_tree_print_tree(root);
    }
    get_time(&t2);

    fprintf(stderr, "%i %f\n", num_keys, elapsed_time(&t1, &t2));

    //interval_merge_tree_print_tree(root);

    /* print the tree */
    //fprintf(stderr, "final tree:\n");
    //interval_merge_tree_print_tree(root);

    /* cleanup the tree */
    interval_merge_tree_destroy_tree(root);

    /* destroy the keys */
    for(i = 0 ; i < num_keys ; i++)
    {
        free(keys[i].key);
        //interval_merge_tree_destroy_key(keys[i]);
    }
    free(keys);

    return 0;
}
