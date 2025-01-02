// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/asio/spawn.hpp>

#include <functional>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      //! A `::boost::asio::yield_context` wrapper that allows to
      //! execute a function before yielding and resuming the
      //! coroutine, after starting an asynchronous operation and
      //! before returning from one.
      //!
      //! Use this by wrapping the yield context at hand before
      //! passing into an async operation, e.g.
      //!
      //!     ::boost::asio::yield_context yield;
      //!     std::lock_guard<std::mutex> const lock (mutex);
      //!     ::boost::asio::async_write ( _socket
      //!                              , buffers
      //!                              , yield_context_with_hooks
      //!                                  { yield
      //!                                  , [&] { mutex.unlock(); }
      //!                                  , [&] { mutex.lock(); }
      //!                                  }
      //!                              );
      struct yield_context_with_hooks
      {
        ::boost::asio::yield_context yield_context;
        std::function<void()> before_yield;
        std::function<void()> before_resume;
      };
    }
  }
}

#include <util-rpc/detail/yield_context_with_hooks.ipp>
