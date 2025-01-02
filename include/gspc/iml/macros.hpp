// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
