// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/state.hpp>

#include <we/type/property.hpp>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      namespace property
      {
        void set_state ( state::type& state
                       , we::type::property::type& prop
                       , we::type::property::path_type const& path
                       , we::type::property::value_type const& value
                       );
        void join ( state::type const& state
                  , we::type::property::type& x
                  , we::type::property::type const& y
                  );
      }
    }
  }
}
