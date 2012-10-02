#pragma once

#include "LuaBridge/RefCountedObject.h"

namespace pnetopt {

template<class Range>
class LuaIterator: public pnetopt::Invalidatable, public RefCountedObjectType<int> {
    Range range_;
    typename Range::const_iterator current_;
    typename Range::const_iterator end_;

    public:

    LuaIterator(const Range &range):
        range_(range), current_(range.begin()), end_(range.end())
    {}

    const char *toString() {
        ensureValid();
        return "LuaIterator<>";
    }

    typename Range::value_type call() {
        ensureValid();

        if (current_ != end_) {
            return *current_++;
        } else {
            return NULL;
        }
    };

    typename Range::size_type size() const {
        return range_.size();
    }
};

} // namespace pnetopt

/* vim:set et sts=4 sw=4: */
