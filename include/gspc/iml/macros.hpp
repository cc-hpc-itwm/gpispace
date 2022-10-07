// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#if GSPC_WITH_IML
  #define IF_GSPC_WITH_IML(parameter) parameter
#else
  #define IF_GSPC_WITH_IML(parameter)
#endif

namespace gspc
{
  template<typename...>
  struct WithIML
  {
    static constexpr bool value {static_cast<bool>(GSPC_WITH_IML)};
  };

  template<typename... Args>
  constexpr auto WithIML_v = WithIML<Args...>::value;
}

#define GSPC_WITHOUT_IML_ERROR_MESSAGE              \
"\n"                                                \
"\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" \
"\n!! GPI-Space installation has no IML support !!" \
"\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" \
"\n"

#define GSPC_WITHOUT_IML_API_ERROR(name)         \
template<typename... Args>                       \
auto name (Args&&...)                            \
{                                                \
  static_assert ( gspc::WithIML_v<Args...>       \
                , GSPC_WITHOUT_IML_ERROR_MESSAGE \
                );                               \
}
