// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/serialization/split_free.hpp>
#include <optional>

namespace boost::serialization
{
  template<typename Archive, typename T>
    void load
      (Archive& ar, std::optional<T>& optional, const unsigned int)
  {
    auto has_value {bool{}};
    ar >> has_value;
    if (has_value)
    {
      auto x {T{}};
      ar >> x;
      optional = x;
    }
  }
  template<typename Archive, typename T>
    void save
      (Archive& ar, std::optional<T> const& optional, const unsigned int)
  {
    if (optional.has_value())
    {
      ar << true << *optional;
    }
    else
    {
      ar << false;
    }
  }

  template<typename Archive, typename T>
    void serialize
     ( Archive& ar
     , std::optional<T>& optional
     , const unsigned int version
     )
  {
    split_free (ar, optional, version);
  }
}
