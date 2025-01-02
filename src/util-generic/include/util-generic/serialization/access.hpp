// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//! Grant friend-access to util-generic's serialization functions,
//! namely by_member.
#define FHG_UTIL_SERIALIZATION_ACCESS(type_)                            \
  FHG_UTIL_SERIALIZATION_ACCESS_IMPL (type_)

#include <util-generic/serialization/access.ipp>
