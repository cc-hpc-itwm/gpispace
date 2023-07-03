// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
