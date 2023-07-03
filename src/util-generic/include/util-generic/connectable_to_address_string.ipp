// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace fhg
{
  namespace util
  {
    template<typename BoostAsioIpEndpoint>
      std::pair<std::string, unsigned short>
        connectable_to_address_string (BoostAsioIpEndpoint endpoint)
    {
      return { connectable_to_address_string (endpoint.address())
             , endpoint.port()
             };
    }
  }
}
