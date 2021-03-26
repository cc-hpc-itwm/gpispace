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

#include <boost/asio/spawn.hpp>

#include <functional>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      //! A `boost::asio::yield_context` wrapper that allows to
      //! execute a function before yielding and resuming the
      //! coroutine, after starting an asynchronous operation and
      //! before returning from one.
      //!
      //! Use this by wrapping the yield context at hand before
      //! passing into an async operation, e.g.
      //!
      //!     boost::asio::yield_context yield;
      //!     std::lock_guard<std::mutex> const lock (mutex);
      //!     boost::asio::async_write ( _socket
      //!                              , buffers
      //!                              , yield_context_with_hooks
      //!                                  { yield
      //!                                  , [&] { mutex.unlock(); }
      //!                                  , [&] { mutex.lock(); }
      //!                                  }
      //!                              );
      struct yield_context_with_hooks
      {
        boost::asio::yield_context yield_context;
        std::function<void()> before_yield;
        std::function<void()> before_resume;
      };
    }
  }
}

#include <rpc/detail/yield_context_with_hooks.ipp>
