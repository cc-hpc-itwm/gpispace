// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/functor_visitor.hpp>

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      template<typename T, typename... Ts>
        bool holds_alternative (::boost::variant<Ts...> const& variant) noexcept
      {
        return visit<bool>
          ( variant
          , [] (T const&) { return true; }
          , [] (auto const&) { return false; }
          );
      }
    }
  }
}
