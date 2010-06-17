#ifndef MERGE_RB_INTERVAL_TREE_HH
#define MERGE_RB_INTERVAL_TREE_HH

#include "c-util/interval-merge-tree.h"

namespace iofwdutil
{
    namespace rm 
    {
        int IntervalMergeTreeIntervalLLInit(interval_merge_tree_node_t * node, void * data);
        int IntervalMergeTreeIntervalLLDestroy(interval_merge_tree_interval_ll_t * ll);
        interval_merge_tree_node_t * IntervalMergeTreeCreateNode();
        int IntervalMergeTreeDestroyNode(interval_merge_tree_node_t * node);
        int IntervalMergeTreeDestroyTree(interval_merge_tree_node_t * node);
        int IntervalMergeTreeInsert(interval_merge_tree_node_t ** node, interval_merge_tree_key_t key, interval_merge_tree_node_t ** root);
        int IntervalMergeTreePrintTree(interval_merge_tree_node_t * node);
        int IntervalMergeTreePrint(interval_merge_tree_node_t * node, int level, char dir);
        interval_merge_tree_key_t IntervalMergeTreeKeyFind(interval_merge_tree_node_t * node, size_t k);
        interval_merge_tree_node_t * IntervalMergeTreeNodeFind(interval_merge_tree_node_t * node, size_t k);
        interval_merge_tree_node_t * IntervalMergeTreeDelete(interval_merge_tree_node_t ** root, interval_merge_tree_node_t * node);
        interval_merge_tree_node_t * IntervalMergeTreeFindDelete(interval_merge_tree_node_t ** root, size_t k);
        interval_merge_tree_node_t * IntervalMergeTreeNodeFindRank(interval_merge_tree_node_t * node, size_t k, int op);
        interval_merge_tree_node_t * IntervalMergeTreeIntervalSearch(interval_merge_tree_node_t * root, interval_merge_tree_interval_t interval, unsigned int * oc);
        int IntervalMergeTreeMergeIntervals(interval_merge_tree_node_t ** root, interval_merge_tree_node_t ** n_node, interval_merge_tree_key_t key);
        size_t IntervalMergeTreeSize(interval_merge_tree_node_t * node);
        interval_merge_tree_node_t * IntervalMergeTreeDeleteMinNode(interval_merge_tree_node_t ** root);
    }
}
#endif
