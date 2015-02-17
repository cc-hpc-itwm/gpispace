// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <boost/functional/hash.hpp>

#include <functional>
#include <utility>

namespace std
{
  template<typename T1, typename T2>
    struct hash<std::pair<T1, T2>>
  {
    std::size_t operator() (const std::pair<T1, T2>& p) const
    {
      size_t seed = 0;
      boost::hash_combine (seed, p.first);
      boost::hash_combine (seed, p.second);
      return seed;
    }
  };
}
