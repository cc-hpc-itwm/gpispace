// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <set>
#include <unordered_map>

namespace fhg
{
  namespace util
  {
    template<typename T>
      class refcounted_set
    {
    public:
      void emplace (T x)
      {
        ++_count[x];
        _xs.emplace (std::move (x));
      }
      void erase (T const& x)
      {
        if (--_count.at (x) == 0)
        {
          _xs.erase (x);
          _count.erase (x);
        }
      }
      bool contains (T const& x) const
      {
        return _xs.count (x);
      }
      operator std::set<T> const&() const
      {
        return _xs;
      }

    private:
      std::set<T> _xs;
      std::unordered_map<T, std::size_t> _count;
    };
  }
}
