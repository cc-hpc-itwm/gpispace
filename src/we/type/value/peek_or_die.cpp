// Copyright (C) 2023-2024 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/value/show.formatter.hpp>
#include <fmt/core.h>
#include <gspc/we/type/value/path/join.hpp>
#include <gspc/we/type/value/peek_or_die.hpp>
#include <gspc/we/type/value/show.hpp>

namespace gspc::pnet::type::value::error
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
