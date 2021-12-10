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

#include <testing/hopefully_free_port.hpp>

namespace test
{
  hopefully_free_port::hopefully_free_port()
    : _io_service()
    , _dispatcher()
    , _provider ( std::make_unique
                    <fhg::rpc::service_tcp_provider_with_deferred_start>
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
