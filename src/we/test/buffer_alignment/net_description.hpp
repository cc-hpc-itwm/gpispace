// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace we
{
  namespace test
  {
    namespace buffer_alignment
    {
      struct BufferInfo
      {
        std::string name;
        unsigned long size;
        ::boost::optional<unsigned long> alignment;
      };

      std::string create_net_description (std::vector<BufferInfo> const&);
    }
  }
}
