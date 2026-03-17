// Copyright (C) 2018,2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/hopefully_free_port.hpp>

namespace gspc::testing
{
  hopefully_free_port::hopefully_free_port()
    : _io_service()
    , _dispatcher()
    , _provider ( std::make_unique
                    <gspc::rpc::service_tcp_provider_with_deferred_start>
                      (_io_service, _dispatcher)
                )
    , _port (_provider->local_endpoint().port())
  {}

  hopefully_free_port::operator unsigned short() const
  {
    return _port;
  }
  unsigned short hopefully_free_port::release()
  {
    _provider.reset();
    return *this;
  }
}
