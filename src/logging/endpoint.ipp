// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/serialization/optional.hpp>

namespace fhg
{
  namespace logging
  {
    template<typename Archive>
      void serialize (Archive& ar, endpoint& ep, unsigned int const)
    {
      ar & ep.as_tcp;
      ar & ep.as_socket;
    }
  }
}
