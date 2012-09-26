#pragma once

#include <typeinfo>

#include <boost/format.hpp>

namespace pnetopt {

class Invalidatable {
    bool valid_;

    public:

    Invalidatable(): valid_(true) {}

    virtual ~Invalidatable() {}

    bool valid() const { return valid_; }

    void invalidate() {
        if (valid_) {
            valid_ = false;
            doInvalidate();
        }
    }

    void ensureValid() const {
        if (!valid()) {
            throw std::runtime_error((boost::format("accessing an invalidated instance of %1%") % typeid(*this).name()).str());
        }
    }

    bool equals(const Invalidatable *other) const {
        return this == other;
    }

    protected:

    virtual void doInvalidate() {}
};

} // namespace pnetopt

/* vim:set et sts=4 sw=4: */
