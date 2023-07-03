// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <boost/version.hpp>

#ifdef __cplusplus
extern "C" {
#endif

#define WE_MAKE_SYMBOL_NAME(PRE,POST)                                          \
  PRE ## POST
#define WE_MAKE_SYMBOL(BASE,TAG)                                               \
  WE_MAKE_SYMBOL_NAME(BASE,TAG)
#define WE_GUARD_SYMBOL                                                        \
  WE_MAKE_SYMBOL(MODULE_DETECTED_INCOMPATIBLE_BOOST_VERSION_,BOOST_VERSION)

  extern GSPC_DLLEXPORT const unsigned int WE_GUARD_SYMBOL;

#ifdef __cplusplus
}
#endif
