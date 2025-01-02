// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/testing/random/char.hpp>
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
          struct random_impl<std::string>
        {
          //! [a-zA-Z_][a-zA-Z_0-9]*
          struct identifier{};
          //! [a-zA-Z][a-zA-Z_0-9]*
          struct identifier_without_leading_underscore{};
          //! Content of XML tags, i.e. `[^\0<>\\]` and no strings
          //! consisting only of whitespace.
          struct xml_content{};
          //! 0x00..0xFF, except the given \a chars.
          static std::string except (std::string const& chars);
          //! \see random<char>::any()
          //! \see random<char>::any_without_zero()

          static constexpr std::size_t const default_max_length = 1 << 10;

          //! Generate a random string using only the characters given
          //! in \a chars, with a random length between \a min_length
          //! and \a max_length (inclusive).
          std::string operator()
            ( std::string const& chars = random_impl<char>::any()
            , std::size_t max_length = default_max_length
            , std::size_t min_length = 0
            ) const;

          //! Generate a random identifier string with a random length
          //! between \a min_length and \a max_length (inclusive).
          std::string operator()
            ( identifier
            , std::size_t max_length = default_max_length
            , std::size_t min_length = 1
            ) const;
          //! Generate a random identifier (without leading
          //! underscore) string with a random length between \a
          //! min_length and \a max_length (inclusive).
          std::string operator()
            ( identifier_without_leading_underscore
            , std::size_t max_length = default_max_length
            , std::size_t min_length = 1
            ) const;
          //! Generate a random xml tag content string that can be
          //! used as text content without CDATA, with a random length
          //! between \a min_length and \a max_length (inclusive).
          std::string operator()
            ( xml_content
            , std::size_t max_length = default_max_length
            , std::size_t min_length = 0
            ) const;
        };
      }

      //! random string of up to \a max_length (default <= 2^10),
      //! character uniform from \a chars.
      //! \note: deprecated Use `random<std::string>{} (chars)` or
      //! `random<std::string>{} (chars, max_length)` instead.
      std::string random_string_of ( std::string const& chars
                                   , int max_length = (1 << 10)
                                   );

      //! equivalent to random_string_of (map char [0..255])
      //! \note: deprecated Use `random<std::string>{}()` instead.
      std::string random_string();

      //! equivalent to
      //! random_string_of (filter (not . elem except) . map char [0..255])
      //! \note: deprecated Use `random<std::string>{}
      //! (random<std::string>::except (except));` instead.
      std::string random_string_without (std::string const& except);

      //! [a-zA-Z_][a-zA-Z_0-9]*
      //! \note: deprecated Use `random<std::string>{}
      //! (random<std::string>::identifier{})` or
      //! `random<std::string>{} (random<std::string>::identifier{},
      //! max_length)` instead.
      std::string random_identifier (int max_length = (1 << 10));

      //! [a-zA-Z][a-zA-Z_0-9]*
      //! \note: deprecated Use `random<std::string>{}
      //! (random<std::string>::identifier_without_leading_underscore{})`
      //! instead.
      std::string random_identifier_without_leading_underscore();

      //! content of xml tags, e.g. [^\0<>\\]
      //! \note: deprecated Use `random<std::string>{}
      //! (random<std::string>::content_string())` instead.
      std::string random_content_string();

      //! \note that std::string::operator+ has problems with the zero
      //! \note: deprecated Use `random<std::string>{}
      //! (random<char>::any_without_zero());` instead.
      std::string random_string_without_zero();
    }
  }
}
