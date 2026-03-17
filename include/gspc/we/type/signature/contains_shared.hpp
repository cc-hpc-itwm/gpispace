// Copyright (C) 2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/signature.hpp>

namespace gspc::pnet::type::signature
{
  // Check if a signature contains any shared type anywhere in its
  // structure. Used for auto-detection of shared tracking
  // requirements.
  //
  bool contains_shared (signature_type const&);
}
