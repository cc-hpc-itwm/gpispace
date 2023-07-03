// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
