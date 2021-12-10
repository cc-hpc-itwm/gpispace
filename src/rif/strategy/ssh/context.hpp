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

#include <boost/asio/ssl/detail/openssl_init.hpp>

namespace libssh2
{
  //! \todo not thread safe
  struct context
  {
    context();
    ~context();

  private:
    // Some targets link both, this file and Boost.Asio's SSL. This
    // leads to the issue that two locations try to set the OpenSSL
    // global state. By re-using the Boost.Asio implementation detail,
    // this is avoided. (also see issue #912)
    // This is **not** public API, it is a detail of
    // `::boost::asio::ssl::context`, which has the same member.
    ::boost::asio::ssl::detail::openssl_init<> init_;
  };
}
