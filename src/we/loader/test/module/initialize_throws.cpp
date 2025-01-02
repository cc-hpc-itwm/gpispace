// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/loader/macros.hpp>

#include <stdexcept>

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif

WE_MOD_INITIALIZE_START()
{
  throw std::runtime_error ("initialize_throws");
}

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

WE_MOD_INITIALIZE_END()
