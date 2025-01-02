// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/signature.hpp>

#include <string>
#include <unordered_map>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      void specialize ( structured_type&
                      , std::unordered_map<std::string, std::string> const&
                      );
    }
  }
}
