// Copyright (C) 2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/com/peer.hpp>


  namespace gspc::com
  {
    //! connection to one other end, synchrounous send
    class channel : private peer_t
    {
    public:
      channel ( std::unique_ptr<::boost::asio::io_service>
              , port_t const& port
              , gspc::Certificates const& certificates
              , host_t const& other_end_host
              , port_t const& other_end_port
              );
      void send (std::string const&);
      p2p::address_t const& other_end() const;

      using peer_t::async_recv;
      using peer_t::address;

    private:
      p2p::address_t _other_end;
    };
  }
