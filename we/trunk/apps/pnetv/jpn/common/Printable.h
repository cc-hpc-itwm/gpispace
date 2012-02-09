#pragma once

#include <iosfwd>
#include <string>

namespace jpn {

/**
 * Base class for objects printable into streams.
 */
class Printable {
    public:

    /**
     * Virtual destructor.
     */
    virtual ~Printable() {}

    /**
     * Prints the object into a stream in a human-readable form.
     */
    virtual void print(std::ostream &out) const = 0;

    std::string toString() const;
};

inline
std::ostream &operator<<(std::ostream &out, const Printable &object) {
    object.print(out);
    return out;
}

} // namespace jpn

/* vim:set et sts=4 sw=4: */
