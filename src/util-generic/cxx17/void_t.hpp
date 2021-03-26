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

#pragma once

#if HAS_STD_VOID_T

#include <type_traits>

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      using std::void_t;
    }
  }
}

#else

//! \note n3911: TransformationTrait Alias `void_t`

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      template<typename...> struct make_void { using type = void; };
      template<typename... Ts> using void_t
        = typename make_void<Ts...>::type;
    }
  }
}

#endif
