// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//! A function or type marked with this macro is exported into the
//! public binary API, when producing a dynamically linked binary.
#define FHG_UTIL_DLLEXPORT FHG_UTIL_DLLEXPORT_IMPL

#include <util-generic/dllexport.ipp>
