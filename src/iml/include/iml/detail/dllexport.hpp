// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//! A function or type marked with this macro is exported into the
//! public binary API.
#define IML_DLLEXPORT IML_DLLEXPORT_IMPL

#include <iml/detail/dllexport.ipp>
