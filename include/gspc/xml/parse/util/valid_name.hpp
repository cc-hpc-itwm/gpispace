// Copyright (C) 2010,2012-2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/parse/position.hpp>

#include <filesystem>

#include <string>


  namespace gspc::xml::parse
  {
    //! \note [:space:]*([:alpha:_][:alpha::num:_]*)[:space:]*
    std::string parse_name (gspc::util::parse::position& pos);

    //! \note parse_name (name) == name
    std::string validate_name ( std::string const& name
                              , std::string const& type
                              , std::filesystem::path const& path
                              );
  }
