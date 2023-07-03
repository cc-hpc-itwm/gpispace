// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/ostream/redirect.hpp>

#include <functional>
#include <ostream>
#include <streambuf>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      redirect::redirect ( std::ostream& os
                         , std::function<void (std::string const&)> line
                         )
        : std::streambuf()
        , _os (os)
        , _line (line)
        , _buffer()
        , _streambuf (_os.rdbuf (this))
      {}

      redirect::~redirect()
      {
        if (_streambuf)
        {
          _os.rdbuf (_streambuf);
        }

        if (!_buffer.empty())
        {
          _line (_buffer);
        }
      }

      auto redirect::overflow (int_type c) -> int_type
      {
        if ('\n' == traits_type::to_char_type (c))
        {
          _os.rdbuf (_streambuf);
          _streambuf = nullptr;
          _line (_buffer);
          _streambuf = _os.rdbuf (this);

          _buffer.clear();
        }
        else
        {
          _buffer += traits_type::to_char_type (c);
        }

        return c;
      }

      prepend_line::prepend_line
          ( std::ostream& os
          , std::function<std::string (std::string const&)> prefix
          )
        : redirect ( os
                   , [&os, prefix] (std::string const& line)
                     {
                       os << prefix (line) << line << '\n';
                     }
                   )
      {}
      prepend_line::prepend_line ( std::ostream& os
                                 , std::function<std::string ()> prefix
                                 )
        : prepend_line (os, [prefix] (std::string const&) { return prefix(); })
      {}
      prepend_line::prepend_line (std::ostream& os, std::string prefix)
        : prepend_line (os, [prefix] () { return prefix; })
      {}
    }
  }
}
