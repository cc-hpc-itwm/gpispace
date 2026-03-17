// Copyright (C) 2010,2013-2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/expr/parse/parser.hpp>

#include <gspc/xml/parse/error.hpp>

#include <string>



    namespace gspc::xml::parse::util
    {
      gspc::we::expr::parse::parser generic_we_parse ( std::string const& input
                                           , std::string const& descr
                                           );
      gspc::we::expr::parse::parser we_parse ( std::string const& input
                                   , std::string const& descr
                                   , std::string const& type
                                   , std::string const& name
                                   , std::filesystem::path const&
                                   );
   }
