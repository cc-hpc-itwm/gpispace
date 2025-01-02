// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/signature.hpp>

#include <string>
#include <unordered_set>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      std::unordered_set<std::string> names (signature_type const&);
    }
  }
}
