// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <fhg/util/starts_with.hpp>

#include <string>

namespace rewrite
{
  static std::string magic_prefix = "_";

  inline bool has_magic_prefix (std::string const& name)
  {
    return fhg::util::starts_with (magic_prefix, name);
  }

  inline std::string mk_prefix (std::string const& name)
  {
    return magic_prefix + name + magic_prefix;
  }
}
