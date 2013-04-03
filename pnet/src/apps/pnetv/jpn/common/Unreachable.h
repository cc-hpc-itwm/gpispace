#pragma once

#include <cassert>
#include <fhg/assert.hpp>

namespace jpn {

inline void unreachable() {
    assert("NEVER REACHED");
    while (true) {}
}

} // namespace jpn

/* vim:set et sts=4 sw=4: */
