// Copyright (C) 2013-2014,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/util/validprefix.hpp>

#include <gspc/xml/parse/error.hpp>

#include <gspc/xml/parse/rewrite/validprefix.hpp>


  namespace gspc::xml::parse
  {
    std::string validate_prefix ( std::string const& name
                                , std::string const& type
                                , std::filesystem::path const& path
                                )
    {
      if (rewrite::has_magic_prefix (name))
        {
          throw error::invalid_prefix (name, type, path);
        }

      return name;
    }
  }
