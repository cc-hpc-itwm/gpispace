// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <boost/asio/io_service.hpp>
#include <boost/asio/spawn.hpp>

#include <exception>
#include <memory>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      template<typename T>
        struct promise_future_shared_state;
    }
    template<typename T> struct promise;
    template<typename T> struct future;

    //! A Boost.Asio-coroutine-variant of \c std::promise<T>.
    //! \note The implementation uses a UNIX stream socket to
    //! communicate between promise and future. This implies two file
    //! descriptors being used per promise. Futures share state with
    //! the promise and add no additional resource costs.
    template<typename T>
      struct promise
    {
      promise (::boost::asio::io_service&);

      template<typename... U>
        void set_value (U&&...);
      void set_exception (std::exception_ptr);

      future<T> get_future();

      promise (promise<T> const&) = delete;
      promise (promise<T>&&) = delete;
      promise& operator= (promise<T> const&) = delete;
      promise& operator= (promise<T>&&) = delete;
      ~promise() = default;

    private:
      friend struct future<T>;

      std::shared_ptr<detail::promise_future_shared_state<T>> _state;
    };

    //! A Boost.Asio-coroutine-variant of \c std::future<T>.
    //! Pass as template argument to \c remote_function to use a \c
    //! remote_function e.g. within a \c service_handler's callback.
    template<typename T>
      struct future
    {
      //! Wait until the corresponding \c promise has been set.
      T get();
      //! Wait until the corresponding \c promise has been set. Does
      //! not block but yields if the promise was not yet set.
      T get (::boost::asio::yield_context);

      future (future<T> const&) = delete;
      future (future<T>&&) noexcept = default;
      future& operator= (future<T> const&) = delete;
      future& operator= (future<T>&&) = delete;
      ~future() = default;

    private:
      friend struct promise<T>;

      future (std::shared_ptr<detail::promise_future_shared_state<T>>);
      std::shared_ptr<detail::promise_future_shared_state<T>> _state;
    };
  }
}

#include <util-rpc/future.ipp>
