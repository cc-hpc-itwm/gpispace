// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/detail/dllexport.hpp>

#include <string>
#include <vector>

namespace iml
{
  namespace rif
  {
    //! Names of strategies available for managing RIF daemons.
    IML_DLLEXPORT std::vector<std::string> available_strategies();
  }
}
