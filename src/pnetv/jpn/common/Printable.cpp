#include "Printable.h"

#include <sstream>

namespace jpn {

std::string Printable::toString() const {
    std::stringstream out;
    print(out);
    return out.str();
}

} // namespace jpn
