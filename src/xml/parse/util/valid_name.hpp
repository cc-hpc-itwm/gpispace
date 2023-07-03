// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <fhg/util/parse/position.hpp>

#include <boost/filesystem/path.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    //! \note [:space:]*([:alpha:_][:alpha::num:_]*)[:space:]*
    std::string parse_name (fhg::util::parse::position& pos);

    //! \note parse_name (name) == name
    std::string validate_name ( std::string const& name
                              , std::string const& type
                              , ::boost::filesystem::path const& path
                              );
  }
}
