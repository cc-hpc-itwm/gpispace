#pragma once

#include <gspc/util/hash/combined_hash.hpp>

#include <functional>
#include <utility>

namespace std
{
  template<typename T1, typename T2>
    struct hash<std::pair<T1, T2>>
  {
    std::size_t operator() (std::pair<T1, T2> const& p) const
    {
      return gspc::util::combined_hash (p.first, p.second);
    }
  };
}
