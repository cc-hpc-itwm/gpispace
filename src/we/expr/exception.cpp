// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/expr/exception.hpp>

namespace expr
{
  namespace exception
  {
    namespace parse
    {
      exception::exception (std::string const& msg, std::size_t k)
        : std::runtime_error {fmt::format ("parse error [{}]: {}", k, msg)}
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
          { fmt::format ( "unterminated {}, opened at: {}"
                        , what
                        , open
                        )
          , k
          }
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

      negative_exponent::negative_exponent()
        : std::runtime_error ("negative exponent")
      {}
    }

    namespace type
    {
      error::error (std::string const& msg)
        : std::runtime_error ("type error: " + msg)
      {}
    }
  }
}
