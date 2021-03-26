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

#pragma once

#include <rpc/service_dispatcher.hpp>
#include <rpc/service_tcp_provider.hpp>

#include <boost/asio/io_service.hpp>

#include <memory>

namespace test
{
  //! When needing fixed ports for tests, actually hardcoding them
  //! would be error prone. This utility constructs a TCP RPC provider
  //! with automatic port assignment, which then is destructed upon
  //! `release()` right before creating the real resource with the
  //! port, in the hope that no other thread or process gets the port
  //! assigned in the meantime.
  //! \note This may still end in failed tests! The ports are
  //! *hopefully* free, not guaranteed.
  //! \note This is probably better put into util-generic, but it uses
  //! RPC, so that would be a circular dependency. Thus, this helper
  //! exists in copies in multiple projects.
  struct hopefully_free_port
  {
    hopefully_free_port();

    operator unsigned short() const;
    unsigned short release();

  private:
    boost::asio::io_service _io_service;
    fhg::rpc::service_dispatcher _dispatcher;
    std::unique_ptr<fhg::rpc::service_tcp_provider_with_deferred_start>
      _provider;
    unsigned short const _port;
  };
}
