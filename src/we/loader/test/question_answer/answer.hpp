// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/dllexport.hpp>

extern "C"
{
  FHG_UTIL_DLLEXPORT extern long the_answer;
  FHG_UTIL_DLLEXPORT long get_answer();
}
