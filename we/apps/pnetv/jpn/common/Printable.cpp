#include "Printable.h"

#include <sstream>

namespace jpn {

std::string Printable::toString() const {
    std::stringstream out;
    print(out);
    return out.str();
}

} // namespace jpn

/* vim:set et sts=4 sw=4: */
