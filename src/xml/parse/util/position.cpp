// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/util/position.hpp>

#include <iostream>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      namespace
      {
        void inc_line_and_column ( const char* pos
                                 , const char* end
                                 , unsigned int& line
                                 , unsigned int& column
                                 )
        {
          for (; pos != end; ++pos)
          {
            ++column;

            if (*pos == '\n')
            {
              column = 0;
              ++line;
            }
          }
        }
      }

      position_type::position_type ( const char* begin
                                   , const char* pos
                                   , ::boost::filesystem::path const& path
                                   , unsigned int const& line
                                   , unsigned int const& column
                                   )
        : _line (line)
        , _column (column)
        , _path (path)
      {
        inc_line_and_column (begin, pos, _line, _column);
      }

      const unsigned int& position_type::line() const
      {
        return _line;
      }
      const unsigned int& position_type::column() const
      {
        return _column;
      }
      ::boost::filesystem::path const& position_type::path() const
      {
        return _path;
      }

      std::ostream& operator<< (std::ostream& os, position_type const& p)
      {
        return os << "[" << p.path().string()
                  << ":" << p.line()
                  << ":" << p.column()
                  << "]";
      }
    }
  }
}
