// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
            ((::boost::format ("parse error [%1%]: %2%") % k % msg).str())
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
          ( ( ::boost::format ("unterminated %1%, opened at: %2%")
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
      type_error::type_error (::boost::format const& msg)
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
      error::error (::boost::format const& msg)
        : error (msg.str())
      {}
    }
  }
}
