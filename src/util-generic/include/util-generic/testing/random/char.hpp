// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/testing/random/impl.hpp>

#include <string>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        template<>
          struct random_impl<char>
        {
          //! 0x00..0xFF
          static std::string const& any();
          //! 0x01..0xFF
          static std::string const& any_without_zero();

          //! Generate a random character choosing from only the
          //! characters given in \a chars.
          char operator() (std::string const& chars = any()) const;
        };
      }

      //! uniformly select a char from \a chars
      //! \note: deprecated Prefer random<char>.
      char random_char_of (std::string const& chars);
    }
  }
}
