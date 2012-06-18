#pragma once

namespace trench {

class Invalidatable {
    const char *message_;
    bool valid_;

    public:

    Invalidatable(const char *message): message_(message), valid_(true) {}

    Invalidatable(): message_("Accessing object in invalid state"), valid_(true) {}

    bool valid() const { return valid_; }

    void invalidate() { valid_ = false; }

    void ensureValid() {
        if (!valid()) {
            throw std::runtime_error(message_);
        }
    }
};

} // namespace trench

/* vim:set et sts=4 sw=4: */
