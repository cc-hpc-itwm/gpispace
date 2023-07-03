// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/dllexport.hpp>

extern "C" FHG_UTIL_DLLEXPORT int dltest_variable;
extern "C" FHG_UTIL_DLLEXPORT void dltest_setter (int);
extern "C" FHG_UTIL_DLLEXPORT int dltest_getter();
