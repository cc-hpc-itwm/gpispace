// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include<boost/variant/variant_fwd.hpp>

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      /**
       * @brief holds_alternative implementation for ::boost::variant
       *
       * This is an implementation of holds_alternative for ::boost::variant.
       * It checks if the given ::boost::variant holds the queried alternative
       * type.
       * The call is ill-formed if the queried type doesn't appear exactly once
       * in the variant's types.
       *
       * @tparam T The type to check within the Variant
       * @tparam Ts Types contained within the ::boost::variant variable
       * @param[in] variant ::boost::variant object to query
       * @return True if T is found exactly once, False otherwise.
       */
      template<typename T, typename... Ts>
        bool holds_alternative (::boost::variant<Ts...> const& variant) noexcept;
    }
  }
}

#include <util-generic/cxx17/holds_alternative.ipp>
