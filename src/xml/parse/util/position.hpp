// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/util/position.fwd.hpp>

#include <boost/filesystem.hpp>

#include <iosfwd>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      class position_type
      {
      public:
        position_type ( const char* begin
                      , const char* pos
                      , ::boost::filesystem::path const&
                      , unsigned int const& line = 1
                      , unsigned int const& column = 0
                      );
        const unsigned int& line() const;
        const unsigned int& column() const;
        ::boost::filesystem::path const& path() const;

      private:
        unsigned int _line;
        unsigned int _column;
        ::boost::filesystem::path _path;
      };

      std::ostream& operator<< (std::ostream&, position_type const&);

#define XML_PARSE_UTIL_POSITION_GENERATED()                             \
      ::xml::parse::util::position_type (nullptr, nullptr, __FILE__, __LINE__)
    }
  }
}
