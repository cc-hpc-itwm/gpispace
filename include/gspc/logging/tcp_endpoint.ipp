// Copyright (C) 2019,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/serialization/utility.hpp>


  namespace gspc::logging
  {
    template<typename Archive>
      void serialize (Archive& ar, tcp_endpoint& ep, unsigned int const)
    {
      ar & ep.host;
      ar & ep.port;
    }
  }
