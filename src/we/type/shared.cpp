// Copyright (C) 2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/value.hpp>  // Must be before shared.hpp for value_type
#include <gspc/we/type/shared.hpp>

#include <gspc/util/starts_with.hpp>

#include <boost/functional/hash.hpp>

#include <gspc/we/type/value/show.hpp>

#include <ostream>
#include <stdexcept>

namespace gspc::we::type
{
  auto shared::cleanup_place
    ( std::string const& type_name
    ) -> std::optional<std::string>
  {
    static auto const prefix {std::string {"shared_"}};

    if (util::starts_with (prefix, type_name))
    {
      return type_name.substr (prefix.size());
    }

    return {};
  }

  std::ostream& operator<< (std::ostream& os, shared const& s)
  {
    return os << "shared_" << s._cleanup_place
              << " (" << pnet::type::value::show (s.value()) << ")";
  }

  std::size_t hash_value (shared const& s)
  {
    std::size_t seed {0};
    ::boost::hash_combine (seed, hash_value (s.value()));
    ::boost::hash_combine (seed, std::hash<std::string>() (s._cleanup_place));
    return seed;
  }

  bool operator== (shared const& lhs, shared const& rhs)
  {
    return lhs.value() == rhs.value()
        && lhs._cleanup_place == rhs._cleanup_place;
  }

  bool operator!= (shared const& lhs, shared const& rhs)
  {
    return !(lhs == rhs);
  }

  bool operator< (shared const& lhs, shared const& rhs)
  {
    if (lhs._cleanup_place != rhs._cleanup_place)
    {
      return lhs._cleanup_place < rhs._cleanup_place;
    }

    return lhs.value() < rhs.value();
  }

  bool operator<= (shared const& lhs, shared const& rhs)
  {
    return !(rhs < lhs);
  }

  bool operator> (shared const& lhs, shared const& rhs)
  {
    return rhs < lhs;
  }

  bool operator>= (shared const& lhs, shared const& rhs)
  {
    return !(lhs < rhs);
  }
}
