#pragma once

namespace pnetopt {

class Invalidatable {
    const char *message_;
    bool valid_;

    public:

    Invalidatable(const char *message): message_(message), valid_(true) {}

    Invalidatable(): message_("Accessing object in invalid state"), valid_(true) {}

    virtual ~Invalidatable() {}

    bool valid() const { return valid_; }

    void invalidate() {
        valid_ = false;
        doInvalidate();
    }

    void ensureValid() {
        doEnsureValid();
        if (!valid()) {
            throw std::runtime_error(message_);
        }
    }

    protected:

    virtual void doEnsureValid() {}
    virtual void doInvalidate() {}
};

} // namespace pnetopt

/* vim:set et sts=4 sw=4: */
