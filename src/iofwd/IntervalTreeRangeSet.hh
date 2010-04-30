#ifndef IOFWD_INTERVAL_TREE_RANGESET_HH
#define IOFWD_INTERVAL_TREE_RANGESET_HH

#include <cassert>
#include <cstdio>
#include <set>
#include <map>

#include "iofwd/Range.hh"
#include "iofwd/BaseRangeSet.hh"

#include "iofwdutil/rm/IntervalMergeTree.hh"

namespace iofwd
{

class IntervalTreeRangeSet : public BaseRangeSet
{
    public:
        IntervalTreeRangeSet() : rt(NULL)
        {
        }

        ~IntervalTreeRangeSet()
        {
            if(rt)
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

        void get(ChildRange ** r)
        {
            interval_merge_tree_node_t * node = NULL;

            /* delete the node with the min key value */
            /* TODO why not delete the key at the root of the tree? */
            node = iofwdutil::rm::IntervalMergeTreeDeleteMinNode(&rt);

            /* if there is only one range, copy it to r */
            if(node->ll_head == node->ll_tail)
            {
                ChildRange * lr = (ChildRange *) node->ll_head->data;

                /* update the range start and end values */
                *r = lr;
            }
            /* if there is more than one range, create a parent range and copy it to r */
            else
            {
                ParentRange * pr = new ParentRange(node->interval.start, node->interval.end);
                interval_merge_tree_interval_ll_t * cur = node->ll_head;
                ChildRange * lr = (ChildRange *) node->ll_head->data;

                /* update the range values */
                pr->st_ = node->interval.start;
                pr->en_ = node->interval.end;
                pr->buf_ = NULL;
                pr->op_hint_ = lr->op_hint_;
                pr->type_ = lr->type_;
                pr->handle_ = lr->handle_;

                /* add child ranges to this range */
                while(cur)
                {
                    ChildRange * cur_r = (ChildRange *)cur->data;
                    pr->insertSingleChild(cur_r);
                    pr->insertSingleCB(cur_r->cb_);
                    delete (size_t *)cur->key;
                    cur = cur->next;
                }

                /* assign the parent ot the input child range pointer */
                *r = pr;
            }

            /* cleanup */
            iofwdutil::rm::IntervalMergeTreeIntervalLLDestroy(node->ll_head);
            node->ll_head = NULL;
            node->ll_tail = NULL;
            iofwdutil::rm::IntervalMergeTreeDestroyNode(node);
        }

        /* add the range to the interval tree */
        void add(const ChildRange * r)
        {
            int ret = 0;
            interval_merge_tree_node_t * nn = iofwdutil::rm::IntervalMergeTreeCreateNode();
            interval_merge_tree_key_t key;
            size_t * lr_key = new size_t;

            /* init the node */
            *lr_key = r->st_;
            nn->key.key = (void *)lr_key; /* TODO: Fix this... */
            nn->key.value = (void *)lr_key; /* TODO: Fix this... */
            key.key = (void *)lr_key;
            key.value = (void *)lr_key;
            iofwdutil::rm::IntervalMergeTreeIntervalLLInit(nn, (void *)r);
            nn->interval.start = r->st_;
            nn->interval.end = r->en_;
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
