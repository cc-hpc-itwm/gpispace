// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhgcom/channel.hpp>

#include <utility>

namespace fhg
{
  namespace com
  {
    channel::channel ( std::unique_ptr<::boost::asio::io_service> io_service
                     , port_t const& port
                     , Certificates const& certificates
                     , host_t const& other_end_host
                     , port_t const& other_end_port
                     )
      : peer_t (std::move (io_service), host_t {"*"}, port, certificates)
      , _other_end (connect_to (other_end_host, other_end_port))
    {}
    p2p::address_t const& channel::other_end() const
    {
      return _other_end;
    }
    void channel::send (std::string const& data)
    {
      return peer_t::send (_other_end, data);
    }
  }
}
