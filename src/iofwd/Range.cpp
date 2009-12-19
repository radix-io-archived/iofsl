#include "Range.hh"

namespace iofwd
{

Range::Range(ChildRange c)
{
    type = c.type_;
    handle = c.handle_;
    buf = c.buf_;
    st = c.st_;
    en = c.en_;
    op_hint = c.op_hint_;
    cids.push_back(c.cid_);
}

Range::Range(ParentRange p)
{
    type = p.type_;
    handle = p.handle_;
    buf = NULL;
    st = p.st_;
    en = p.en_;
    op_hint = p.op_hint_;

    child_ranges.insert(child_ranges.begin(), p.child_ranges_.begin(), p.child_ranges_.end());
    cids.insert(cids.begin(), p.child_cids_.begin(), p.child_cids_.end());
}

}
