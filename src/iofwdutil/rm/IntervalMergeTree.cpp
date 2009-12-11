#include "IntervalMergeTree.hh"

#include "c-util/interval-merge-tree.h"

namespace iofwdutil 
{
    namespace rm
    {
        int IntervalMergeTreeIntervalLLInit(interval_merge_tree_node_t * node, void * data)
        {
            return interval_merge_tree_interval_ll_init(node, data);
        }

        int IntervalMergeTreeIntervalLLDestroy(interval_merge_tree_interval_ll_t * ll)
        {
            return interval_merge_tree_interval_ll_destroy(ll);
        }

        interval_merge_tree_node_t * IntervalMergeTreeCreateNode()
        {
            return interval_merge_tree_create_node();
        }

        int IntervalMergeTreeDestroyNode(interval_merge_tree_node_t * node)
        {
            return interval_merge_tree_destroy_node(node);
        }

        int IntervalMergeTreeDestroyTree(interval_merge_tree_node_t * node)
        {
            return interval_merge_tree_destroy_tree(node);
        }

        int IntervalMergeTreeInsert(interval_merge_tree_node_t ** node, interval_merge_tree_key_t key, interval_merge_tree_node_t ** root)
        {
            return interval_merge_tree_insert(node, key, root);
        }

        int IntervalMergeTreePrintTree(interval_merge_tree_node_t * node)
        {
            return interval_merge_tree_print_tree(node);
        }

        int IntervalMergeTreePrint(interval_merge_tree_node_t * node, int level, char dir)
        {
            return interval_merge_tree_print(node, level, dir);
        }

        interval_merge_tree_key_t IntervalMergeTreeKeyFind(interval_merge_tree_node_t * node, size_t k)
        {
            return interval_merge_tree_key_find(node, k);
        }

        interval_merge_tree_node_t * IntervalMergeTreeNodeFind(interval_merge_tree_node_t * node, size_t k)
        {
            return interval_merge_tree_node_find(node, k);
        }

        interval_merge_tree_node_t * IntervalMergeTreeDelete(interval_merge_tree_node_t ** root, interval_merge_tree_node_t * node)
        {
            return interval_merge_tree_delete(root, node);
        }

        interval_merge_tree_node_t * IntervalMergeTreeFindDelete(interval_merge_tree_node_t ** root, size_t k)
        {
            return interval_merge_tree_find_delete(root, k);
        }

        interval_merge_tree_node_t * IntervalMergeTreeNodeFindRank(interval_merge_tree_node_t * node, size_t k, int op)
        {
            return interval_merge_tree_node_find_rank(node, k, op);
        }

        interval_merge_tree_node_t * IntervalMergeTreeIntervalSearch(interval_merge_tree_node_t * root, interval_merge_tree_interval_t interval, unsigned int * oc)
        {
            return interval_merge_tree_interval_search(root, interval, oc);
        }

        int IntervalMergeTreeMergeIntervals(interval_merge_tree_node_t ** root, interval_merge_tree_node_t ** n_node, interval_merge_tree_key_t key)
        {
            return interval_merge_tree_merge_intervals(root, n_node, key);
        }

        size_t IntervalMergeTreeSize(interval_merge_tree_node_t * node)
        {
            return interval_merge_tree_size(node);
        }

        interval_merge_tree_node_t * IntervalMergeTreeDeleteMinNode(interval_merge_tree_node_t ** root)
        {
            return interval_merge_tree_delete_min_node(root);
        }
    }
}
