// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#pragma once

#include <limits>
#include <stdexcept>
#include <string>

#if defined(__clang__)
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wvariadic-macros"
 #define COMPILER_INDEPENDENT_PRAGMA_IN_MACRO(...) _Pragma(#__VA_ARGS__)
 #pragma clang diagnostic pop
#elif defined(__GNUG__)
 #define COMPILER_INDEPENDENT_PRAGMA_IN_MACRO(...) _Pragma(#__VA_ARGS__)
#else
 #error Unable to define COMPILER_INDEPENDENT_PRAGMA_IN_MACRO()
#endif

#if defined(__clang__)
 #define DISABLE_WARNING_CLANG(what)                                     \
   COMPILER_INDEPENDENT_PRAGMA_IN_MACRO (clang diagnostic push)          \
   COMPILER_INDEPENDENT_PRAGMA_IN_MACRO (clang diagnostic ignored what)
 #define RESTORE_WARNING_CLANG(what)                                     \
   COMPILER_INDEPENDENT_PRAGMA_IN_MACRO (clang diagnostic pop)
 #define DISABLE_WARNING_GCC(what)
 #define RESTORE_WARNING_GCC(what)
#elif defined(__GNUG__)
 #define DISABLE_WARNING_CLANG(what)
 #define RESTORE_WARNING_CLANG(what)
 #define DISABLE_WARNING_GCC(what)                                       \
   COMPILER_INDEPENDENT_PRAGMA_IN_MACRO (GCC diagnostic push)            \
   COMPILER_INDEPENDENT_PRAGMA_IN_MACRO (GCC diagnostic ignored what)
 #define RESTORE_WARNING_GCC(what)                                       \
   COMPILER_INDEPENDENT_PRAGMA_IN_MACRO (GCC diagnostic pop)
#endif

#if not defined (COMPILER_FLAG_EXISTS_WUNDEFINED_FUNC_TEMPLATE)
# define COMPILER_FLAG_EXISTS_WUNDEFINED_FUNC_TEMPLATE 0
#endif

#if COMPILER_FLAG_EXISTS_WUNDEFINED_FUNC_TEMPLATE
#define DISABLE_WARNING_CLANG_UNDEFINED_FUNC_TEMPLATE()  \
  DISABLE_WARNING_CLANG("-Wundefined-func-template")
#define RESTORE_WARNING_CLANG_UNDEFINED_FUNC_TEMPLATE()  \
  RESTORE_WARNING_CLANG("-Wundefined-func-template")
#else
#define DISABLE_WARNING_CLANG_UNDEFINED_FUNC_TEMPLATE()
#define RESTORE_WARNING_CLANG_UNDEFINED_FUNC_TEMPLATE()
#endif

namespace fhg
{
  namespace util
  {
    namespace suppress_warning
    {
DISABLE_WARNING_CLANG ("-Wsign-conversion")
DISABLE_WARNING_GCC ("-Wsign-conversion")
      template<typename To, typename From>
        To sign_conversion (From&& from, char const* const /*reason*/)
      {
        return from;
      }
RESTORE_WARNING_GCC ("-Wsign-conversion")
RESTORE_WARNING_CLANG ("-Wsign-conversion")

DISABLE_WARNING_CLANG ("-Wsign-compare")
DISABLE_WARNING_GCC ("-Wsign-compare")
      template<typename Lhs, typename Rhs>
        bool sign_compare_lt (Lhs&& lhs, Rhs&& rhs, char const* const /*reason*/)
      {
        return lhs < rhs;
      }
RESTORE_WARNING_GCC ("-Wsign-compare")
RESTORE_WARNING_CLANG ("-Wsign-compare")

DISABLE_WARNING_CLANG ("-Wshorten-64-to-32")
      template<typename To, typename From>
        To shorten_64_to_32_with_check_other
          (From&& from, From&& other, char const* const reason)
      {
        if (other > std::numeric_limits<To>::max())
        {
          throw std::length_error
            (std::string ("shorten_64_to_32 failed: too large (") + reason + ")");
        }
        if ( other < 0
           && sign_compare_lt (other, std::numeric_limits<To>::lowest(), reason)
           )
        {
          throw std::length_error
            (std::string ("shorten_64_to_32 failed: too small (") + reason + ")");
        }
        return from;
      }

      template<typename To, typename From>
        To shorten_64_to_32_with_check (From&& from, char const* const reason)
      {
        return shorten_64_to_32_with_check_other<To> (from, from, reason);
      }
RESTORE_WARNING_CLANG ("-Wshorten-64-to-32")

DISABLE_WARNING_CLANG ("-Wconversion")
      template<typename To, typename From>
        To precision_losing_conversion (From&& from, char const* const /*reason*/)
      {
        return from;
      }
RESTORE_WARNING_CLANG ("-Wconversion")
    }
  }
}
