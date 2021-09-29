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

#include <we/expr/exception.hpp>

#include <boost/format.hpp>

namespace expr
{
  namespace exception
  {
    namespace parse
    {
      exception::exception (std::string const& msg, std::size_t k)
        : std::runtime_error
            ((boost::format ("parse error [%1%]: %2%") % k % msg).str())
        , eaten (k)
      {}

      expected::expected (std::string const& what, std::size_t k)
        : exception ("expected one of: " + what, k)
      {}

      misplaced::misplaced (std::string const& what, std::size_t k)
        : exception ( "misplaced " + what + ", operator expected"
                    , k - what.length()
                    )
      {}

      unterminated::unterminated ( std::string const& what
                                 , std::size_t open
                                 , std::size_t k
                                 )
        : exception
          ( ( boost::format ("unterminated %1%, opened at: %2%")
            % what
            % open
            ).str()
          , k
          )
      {}

      missing::missing (std::string const& what, std::size_t k)
        : exception ("missing " + what, k)
      {}
    }

    namespace eval
    {
      divide_by_zero::divide_by_zero()
        : std::runtime_error ("divide by zero")
      {}

      type_error::type_error (std::string const& msg)
        : std::runtime_error ("type error: " + msg)
      {}
      type_error::type_error (boost::format const& msg)
        : std::runtime_error ("type error: " + msg.str())
      {}

      negative_exponent::negative_exponent()
        : std::runtime_error ("negative exponent")
      {}
    }

    namespace type
    {
      error::error (std::string const& msg)
        : std::runtime_error ("type error: " + msg)
      {}
      error::error (boost::format const& msg)
        : error (msg.str())
      {}
    }
  }
}
