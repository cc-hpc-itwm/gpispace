// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/optional.hpp>
#include <string>

namespace sdpa
{
  namespace daemon
  {
    using Implementation = ::boost::optional<std::string>;
  }
}
