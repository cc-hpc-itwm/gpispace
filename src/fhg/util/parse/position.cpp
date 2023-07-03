// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/parse/position.hpp>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/require.hpp>

#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      position::position (std::string const& input)
        : _pos (input.begin())
        , _begin (input.begin())
        , _end (input.end())
      {}

      char position::operator*() const
      {
        return *_pos;
      }
      void position::operator++()
      {
        ++_k;
        ++_pos;
      }
      bool position::end() const
      {
        return _pos == _end;
      }
      std::size_t position::eaten() const
      {
        return _k;
      }

      std::string position::error_message (std::string const& message) const
      {
        std::ostringstream oss;

        oss << "PARSE ERROR [" << eaten() << "]: " << message << std::endl;
        oss << std::string (_begin, _pos) << ' '
            << std::string (_pos, _end) << std::endl;
        oss << std::string (eaten(), ' ') << "^" << std::endl;

        return oss.str();
      }
    }
  }
}
