// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/asio/ssl/detail/openssl_init.hpp>

namespace libssh2
{
  //! \todo not thread safe
  struct context
  {
    context();
    ~context();
    context (context const&) = delete;
    context (context&&) = delete;
    context& operator= (context const&) = delete;
    context& operator= (context&&) = delete;

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
