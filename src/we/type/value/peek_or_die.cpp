// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <FMT/we/type/value/show.hpp>
#include <fmt/core.h>
#include <we/type/value/path/join.hpp>
#include <we/type/value/peek_or_die.hpp>
#include <we/type/value/show.hpp>

namespace pnet::type::value::error
{
  auto could_not_peek
    ( value_type const& store
    , std::list<std::string> const& path
    ) -> std::logic_error
  {
    return std::logic_error
      { fmt::format
        ( "Could not peek '{}' from '{}'"
        , path::join (path)
        , show (store)
        )
      };
  }
}
