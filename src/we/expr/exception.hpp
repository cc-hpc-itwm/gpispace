// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <fmt/core.h>
#include <stdexcept>

namespace expr
{
  namespace exception
  {
    namespace parse
    {
      class exception : public std::runtime_error
      {
      public:
        const std::size_t eaten;
        exception (std::string const&, std::size_t);
      };

      class expected : public exception
      {
      public:
        expected (std::string const&, std::size_t);
      };

      class misplaced : public exception
      {
      public:
        misplaced (std::string const&, std::size_t);
      };

      class unterminated : public exception
      {
      public:
        unterminated ( std::string const&
                     , std::size_t open
                     , std::size_t k
                     );
      };

      class missing : public exception
      {
      public:
        missing (std::string const&, std::size_t);
      };
    }

    namespace eval
    {
      class divide_by_zero : public std::runtime_error
      {
      public:
        divide_by_zero();
      };

      class type_error : public std::runtime_error
      {
      public:
        type_error (std::string const&);
      };

      class negative_exponent : public std::runtime_error
      {
      public:
        negative_exponent();
      };

      template<typename T>
      class square_root_for_negative_argument : public std::runtime_error
      {
      public:
        square_root_for_negative_argument (T value)
          : std::runtime_error
            { fmt::format ("square root for negative argument '{}'", value)
            }
          , _value (value)
        {}

      private:
        T _value;
      };

      template<typename T>
      class log_for_nonpositive_argument : public std::runtime_error
      {
      public:
        log_for_nonpositive_argument (T value)
          : std::runtime_error
            { fmt::format ("logarithm for nonpositive argument '{}'", value)
            }
          , _value (value)
        {}

      private:
        T _value;
      };
    }

    namespace type
    {
      class error : public std::runtime_error
      {
      public:
        error (std::string const&);
      };
    }
  }
}
