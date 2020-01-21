#pragma once

#include <vector>

namespace gspc
{
  template<typename T>
    std::vector<char> bytes_save (T const&);
  template<typename T>
    T bytes_load (std::vector<char> const&);
}

#include <gspc/serialization.ipp>
