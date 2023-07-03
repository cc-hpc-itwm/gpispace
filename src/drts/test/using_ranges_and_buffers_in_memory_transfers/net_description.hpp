// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/optional.hpp>

#include <string>

namespace drts
{
  namespace test
  {
    std::string net_description
      (std::string const&, ::boost::optional<bool> allow_empty_ranges, bool);
  }
}
