// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/random/string.hpp>

#include <util-generic/testing/random/integral.hpp>
#include <util-generic/warning.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        std::string random_impl<std::string>::except (std::string const& chars)
        {
          std::string remaining;

          for (int i = 0; i < 256; ++i)
          {
            if (std::find (chars.begin(), chars.end(), char (i)) == chars.end())
            {
              remaining.push_back (char (i));
            }
          }

          return remaining;
        }

#define PRECONDITION(cond_)                                             \
        if (!(cond_))                                                   \
        {                                                               \
          throw std::logic_error                                        \
            ("random<std::string>: precondition failed: " #cond_);      \
        }

        std::string random_impl<std::string>::operator()
          ( std::string const& chars
          , std::size_t max_length
          , std::size_t min_length
          ) const
        {
          PRECONDITION (!chars.empty())
          PRECONDITION (max_length >= min_length)

          std::string s (random<std::size_t>{} (max_length, min_length), '\0');
          std::generate ( s.begin(), s.end()
                        , [&] { return random<char>{} (chars); }
                        );
          return s;
        }

        std::string random_impl<std::string>::operator()
          ( identifier
          , std::size_t max_length
          , std::size_t min_length
          ) const
        {
          auto const first_chars
            ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_");
          auto const remaining_chars
            ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");

          PRECONDITION (min_length > 0)
          PRECONDITION (max_length >= min_length)

          return random<char>{} (first_chars)
               + random<std::string>{}
                   (remaining_chars, max_length - 1, min_length - 1);
        }

        std::string random_impl<std::string>::operator()
          ( identifier_without_leading_underscore
          , std::size_t max_length
          , std::size_t min_length
          ) const
        {
          auto const first_chars
            ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
          auto const remaining_chars
            ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");

          PRECONDITION (min_length > 0)
          PRECONDITION (max_length >= min_length)

          return random<char>{} (first_chars)
               + random<std::string>{}
                   (remaining_chars, max_length - 1, min_length - 1);
        }

        namespace
        {
          bool is_only_whitespace (std::string const& s)
          {
            return std::all_of
              (s.begin(), s.end(), [] (unsigned char x) { return std::isspace (x); });
          }
        }

        std::string random_impl<std::string>::operator()
          ( xml_content
          , std::size_t max_length
          , std::size_t min_length
          ) const
        {
          std::string const zero (1, '\0');
          std::string const forbidden (zero + "<>\\");
          std::string const allowed_middle (except (forbidden));

          PRECONDITION (max_length >= min_length)

          std::string roll;
          do
          {
            roll = (*this) (allowed_middle, max_length, min_length);
          }
          while (is_only_whitespace (roll));

          return roll;
        }

#undef PRECONDITION
      }

      std::string random_string_of
        (std::string const& chars, int const max_length)
      {
        if (max_length < 0)
        {
          throw std::invalid_argument ("random_string_of: max_length negative");
        }

        return random<std::string>{}
        ( chars
        , suppress_warning::sign_conversion<std::size_t>
            (max_length, "max lenght is not negative and fits")
        );
      }

      std::string random_string()
      {
        return random<std::string>{} (random<char>::any());
      }

      std::string random_string_without (std::string const& except)
      {
        return random<std::string>{} (random<std::string>::except (except));
      }

      std::string random_identifier (int const max_length)
      {
        if (max_length < 0)
        {
          throw std::invalid_argument
            ("random_identifier: max_length negative");
        }

        return random<std::string>{}
          ( random<std::string>::identifier()
          , suppress_warning::sign_conversion<std::size_t>
              (max_length, "max lenght is not negative and fits")
          );
      }

      std::string random_identifier_without_leading_underscore()
      {
        return random<std::string>{}
          (random<std::string>::identifier_without_leading_underscore());
      }

      std::string random_content_string()
      {
        return random<std::string>{} (random<std::string>::xml_content{});
      }

      std::string random_string_without_zero()
      {
        return random<std::string>{} (random<char>::any_without_zero());
      }
    }
  }
}
