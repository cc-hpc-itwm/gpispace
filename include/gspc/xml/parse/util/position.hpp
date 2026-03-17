// Copyright (C) 2013-2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/util/position.fwd.hpp>

#include <filesystem>

#include <iosfwd>



    namespace gspc::xml::parse::util
    {
      class position_type
      {
      public:
        position_type ( const char* begin
                      , const char* pos
                      , std::filesystem::path const&
                      , unsigned int const& line = 1
                      , unsigned int const& column = 0
                      );
        const unsigned int& line() const;
        const unsigned int& column() const;
        std::filesystem::path const& path() const;

      private:
        unsigned int _line;
        unsigned int _column;
        std::filesystem::path _path;
      };

      std::ostream& operator<< (std::ostream&, position_type const&);

#define XML_PARSE_UTIL_POSITION_GENERATED()                             \
      ::gspc::xml::parse::util::position_type (nullptr, nullptr, __FILE__, __LINE__)
    }
