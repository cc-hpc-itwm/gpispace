// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_tcp_provider.hpp>

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
    ::boost::asio::io_service _io_service;
    fhg::rpc::service_dispatcher _dispatcher;
    std::unique_ptr<fhg::rpc::service_tcp_provider_with_deferred_start>
      _provider;
    unsigned short const _port;
  };
}
