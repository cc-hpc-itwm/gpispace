// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
                      , const boost::filesystem::path&
                      , const unsigned int& line = 1
                      , const unsigned int& column = 0
                      );
        const unsigned int& line() const;
        const unsigned int& column() const;
        const boost::filesystem::path& path() const;

      private:
        unsigned int _line;
        unsigned int _column;
        boost::filesystem::path _path;
      };

      std::ostream& operator<< (std::ostream&, const position_type&);

#define XML_PARSE_UTIL_POSITION_GENERATED()                             \
      ::xml::parse::util::position_type (nullptr, nullptr, __FILE__, __LINE__)
    }
  }
}
