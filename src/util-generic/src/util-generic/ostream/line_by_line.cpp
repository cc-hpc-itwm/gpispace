// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/ostream/line_by_line.hpp>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      line_by_line::line_by_line
          (std::function<void (std::string const&)> const& callback)
        : std::streambuf()
        , _callback (callback)
      {}
      line_by_line::~line_by_line()
      {
        if (!_buffer.empty())
        {
          _callback (_buffer);
        }
      }

      auto line_by_line::overflow (int_type i) -> int_type
      {
        auto const c (traits_type::to_char_type (i));

        if (c == '\n')
        {
          _callback (_buffer);

          _buffer.clear();
        }
        else
        {
          _buffer += c;
        }

        return i;
      }
    }
  }
}
