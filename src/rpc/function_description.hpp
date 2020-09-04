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

#include <util-generic/cxx17/void_t.hpp>

#include <functional>
#include <string>
#include <type_traits>

namespace fhg
{
  namespace rpc
  {
#define FHG_RPC_FUNCTION_DESCRIPTION(name_, signature_...)          \
    struct name_                                                    \
    {                                                               \
      using signature = signature_;                                 \
                                                                    \
      using result_type                                             \
        = typename std::function<signature>::result_type;           \
                                                                    \
      /*! \note This is not used but is a workaround for */         \
      /* -Wunused-local-typedefs for result_type in GCC. */         \
      struct returns_void : std::is_void<result_type> {};           \
    }

    template<typename T>
      using is_function_description = util::cxx17::void_t
        < typename T::signature
        , typename std::enable_if<std::is_function<typename T::signature>::value>::type
        , typename T::result_type
        >;

    template<typename, typename = void>
      struct is_function_description_t : std::false_type
    {};

    template<typename T>
      struct is_function_description_t
        <T, is_function_description<T>> : std::true_type
    {};
  }
}
