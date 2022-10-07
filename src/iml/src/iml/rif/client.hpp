// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#pragma once

#include <iml/rif/EntryPoint.hpp>
#include <iml/rif/protocol.hpp>

#include <util-rpc/remote_function.hpp>
#include <util-rpc/remote_tcp_endpoint.hpp>

namespace fhg
{
  namespace iml
  {
    namespace rif
    {
      using ::iml::rif::EntryPoint;

      class client
      {
      public:
        client ( ::boost::asio::io_service& io_service
               , EntryPoint const& entry_point
               )
          : _endpoint (io_service, entry_point.hostname, entry_point.port)
          , kill (_endpoint)
          , start_vmem (_endpoint)
        {}

      private:
        fhg::rpc::remote_tcp_endpoint _endpoint;

      public:
        rpc::remote_function<protocol::kill> kill;
        rpc::remote_function<protocol::start_vmem> start_vmem;
      };
    }
  }
}
