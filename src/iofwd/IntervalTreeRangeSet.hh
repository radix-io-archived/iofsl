#ifndef IOFWD_INTERVAL_TREE_RANGESET_HH
#define IOFWD_INTERVAL_TREE_RANGESET_HH

#include <cassert>
#include <cstdio>
#include <set>
#include <map>

#include "Range.hh"

#include "iofwdutil/rm/IntervalMergeTree.hh"

namespace iofwd
{

class IntervalTreeRangeSet
{
    public:
        IntervalTreeRangeSet() : rt(NULL)
        {
        }

        ~IntervalTreeRangeSet()
        {
            if(!rt)
            {
                 iofwdutil::rm::IntervalMergeTreeDestroyTree(rt);
            }
        }

        size_t size() const
        {
            return iofwdutil::rm::IntervalMergeTreeSize(rt);
        }

        bool empty() const
        {
            if(iofwdutil::rm::IntervalMergeTreeSize(rt) == 0)
            {
                return true;
            }
            return false;
        }

        void pop_front(Range &r)
        {
            interval_merge_tree_node_t * node = NULL;

            /* delete the node with the min key value */
            /* TODO why not delete the key at the root of the tree? */
            node = iofwdutil::rm::IntervalMergeTreeDeleteMinNode(&rt);

            /* if there is only one range, copy it to r */
            if(node->ll_head == node->ll_tail)
            {
                Range * lr = (Range *) node->ll_head->data;

                /* update the range start and end values */
                r = *lr;
            }
            else
            {
                interval_merge_tree_interval_ll_t * cur = node->ll_head;
                Range * lr = (Range *) node->ll_head->data;

                /* update the range values */
                r.st = node->interval.start;
                r.en = node->interval.end;
                r.buf = NULL;
                r.op_hint = lr->op_hint;
                r.type = lr->type;
                r.handle = lr->handle;

                //fprintf(stderr, "multi range, parent: st = %lu en = %lu\n", r.st, r.en);
                /* add child ranges to this range */
                while(cur)
                {
                    Range cur_r = *(Range *)cur->data;
                    r.child_ranges.push_back(cur_r);
                    r.cids.insert(r.cids.begin(), cur_r.cids.begin(), cur_r.cids.end());
                    delete (size_t *)cur->key;
                    delete (Range *)cur->data; /* delete the Range created during add() */
                    cur = cur->next;
                }
            }

            /* cleanup */
            iofwdutil::rm::IntervalMergeTreeIntervalLLDestroy(node->ll_head);
            node->ll_head = NULL;
            node->ll_tail = NULL;
            iofwdutil::rm::IntervalMergeTreeDestroyNode(node);
        }

        /* add the range to the interval tree */
        void add(const Range& r)
        {
            int ret = 0;
            interval_merge_tree_node_t * nn = iofwdutil::rm::IntervalMergeTreeCreateNode();
            interval_merge_tree_key_t key;
            Range * lr = new Range(r.st, r.en);
            size_t * lr_key = new size_t;

            /* init the local range */
            lr->handle = r.handle;
            lr->type = r.type;
            lr->buf = r.buf;
            lr->child_ranges = r.child_ranges;
            lr->cids = r.cids;

            /* init the node */
            *lr_key = r.st;
            nn->key.key = (void *)lr_key; /* TODO: Fix this... */
            nn->key.value = (void *)lr_key; /* TODO: Fix this... */
            key.key = (void *)lr_key;
            key.value = (void *)lr_key;
            iofwdutil::rm::IntervalMergeTreeIntervalLLInit(nn, (void *)lr);
            nn->interval.start = r.st;
            nn->interval.end = r.en;
            nn->size = 1;
            nn->max = nn->interval.end;

            /* try to insert it */
            ret = iofwdutil::rm::IntervalMergeTreeMergeIntervals(&rt, &nn, key);

            /* if it could not be inserted */
            if(ret)
            {
                iofwdutil::rm::IntervalMergeTreeIntervalLLDestroy(nn->ll_head);
                nn->ll_head = NULL;
                nn->ll_tail = NULL;
                iofwdutil::rm::IntervalMergeTreeDestroyNode(nn);
            }
        }

    private:
        interval_merge_tree_node_t * rt;
};

}

#endif /* IOFWD_INTERVAL_TREE_RANGESET_HH */
