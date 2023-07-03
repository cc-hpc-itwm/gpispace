// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/serialization/utility.hpp>

namespace fhg
{
  namespace logging
  {
    template<typename Archive>
      void serialize (Archive& ar, socket_endpoint& ep, unsigned int const)
    {
      ar & ep.host;

      std::string path;
      if (typename Archive::is_saving{})
      {
        path = ep.socket.path();
      }
      ar & path;
      if (typename Archive::is_loading{})
      {
        ep.socket = fhg::logging::socket_endpoint::Socket (path);
      }
    }
  }
}
