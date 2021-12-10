// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
