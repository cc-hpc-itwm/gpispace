// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/rif/strategies.hpp>

#include <iml/rif/strategy/meta.hpp>

namespace iml
{
  namespace rif
  {
    std::vector<std::string> available_strategies()
    {
      return fhg::iml::rif::strategy::available_strategies();
    }
  }
}
