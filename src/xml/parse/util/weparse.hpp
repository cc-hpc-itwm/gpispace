// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/parse/parser.hpp>

#include <xml/parse/error.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      expr::parse::parser generic_we_parse ( std::string const& input
                                           , std::string const& descr
                                           );
      expr::parse::parser we_parse ( std::string const& input
                                   , std::string const& descr
                                   , std::string const& type
                                   , std::string const& name
                                   , ::boost::filesystem::path const&
                                   );
   }
  }
}
