// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/util/validprefix.hpp>

#include <xml/parse/error.hpp>

#include <xml/parse/rewrite/validprefix.hpp>

namespace xml
{
  namespace parse
  {
    std::string validate_prefix ( std::string const& name
                                , std::string const& type
                                , ::boost::filesystem::path const& path
                                )
    {
      if (rewrite::has_magic_prefix (name))
        {
          throw error::invalid_prefix (name, type, path);
        }

      return name;
    }
  }
}
