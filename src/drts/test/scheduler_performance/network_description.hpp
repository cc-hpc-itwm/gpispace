// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <string>

#include <boost/optional.hpp>

namespace drts
{
  namespace test
  {
    std::string create_network_description
      ( std::string const&
      , boost::optional<std::string> const&
      , boost::optional<std::string> const&
      );
  }
}
