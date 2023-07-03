// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/cxx17/future_error.hpp>
#include <util-generic/finally.hpp>

#include <boost/asio/local/connect_pair.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/optional.hpp>

#include <future>
#include <mutex>
#include <type_traits>
#include <utility>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      // A `promise` is effectively a tri-value variant with a mutex
      // and condition variable: Either there is nothing set yet, a
      // value was set, or an exception was set. This state is shared
      // with one `future` (there is `shared_future` but this is not
      // supported here). Calling `get()` on the `future` checks
      // whether the state was already set and otherwise waits on the
      // condition variable. When setting a value or exception on the
      // `promise`, the condition variable is notified.
      //
      // Since `void` is a valid type to "store" in a promise, an
      // `enum content`, an `exception_ptr` and either a dummy `void*`
      // or `::boost::optional<T>` is used. Two helpers `value_setter`
      // and `value_getter` handle the `void` case.
      //
      // In the Boost.Asio+Coroutine case, `condition_variable` can't
      // be used: it does not support yielding in `wait()`. Instead,
      // `promise_future_shared_state` essentially re-implements a
      // single-waiter/single-notifier condition using a
      // pipe. `wake()` sends a dummy byte, `wait()` receives that
      // byte. By using `async_read()`, that `wait()` can yield.

      enum class content
      {
        // `get()` will wait, `set_x()` are allowed.
        not_yet_set,
        // `get()` will return immediately, `set_x()` throw.
        value,
        exception,
      };

      template<typename T>
        struct promise_future_shared_state
      {
        promise_future_shared_state (::boost::asio::io_service&);

        void wake();
        void wait();
        void wait (::boost::asio::yield_context);

        ::boost::asio::local::stream_protocol::socket _sender;
        ::boost::asio::local::stream_protocol::socket _receiver;

        std::mutex _guard;
        content _content = content::not_yet_set;
        std::exception_ptr _exception;
        // Maximum use count for state-`shared_ptr` is 2, so could use
        // a cheaper smart-pointer. Removing this bool will make
        // `future` a `std::shared_future` equivalent.
        bool _future_created = false;
        typename std::conditional<std::is_void<T>{}, void*, ::boost::optional<T>>::type
          _value;
      };

      template<typename T>
        promise_future_shared_state<T>::promise_future_shared_state (::boost::asio::io_service& io_service)
        : _sender (io_service)
        , _receiver (io_service)
      {
        ::boost::asio::local::connect_pair (_sender, _receiver);
      }

      template<typename T>
        void promise_future_shared_state<T>::wake()
      {
        char data[1] = {0};
        ::boost::asio::write (_sender, ::boost::asio::buffer (data));
      }
      template<typename T>
        void promise_future_shared_state<T>::wait()
      {
        char data[1] = {0};
        ::boost::asio::read (_receiver, ::boost::asio::buffer (data));
      }
      template<typename T>
        void promise_future_shared_state<T>::wait (::boost::asio::yield_context yield)
      {
        char data[1] = {0};
        ::boost::asio::async_read
          (_receiver, ::boost::asio::buffer (data), yield);
      }
    }

    template<typename T>
      promise<T>::promise (::boost::asio::io_service& io_service)
        : _state ( std::make_shared<detail::promise_future_shared_state<T>>
                     (io_service)
                 )
    {}

    // \todo Add promise<T>::~promise() so that a future outliving the
    // promise will throw `broken_promise` on `get()`. Might use
    // `_state.use_count()` to get this implicitly without a new enum
    // value in the state.

    template<typename T>
      struct value_setter
    {
      template<typename Value, typename... U>
        void operator() (Value& v, U&&... x)
      {
        v = T (std::forward<U> (x)...);
      }
    };
    template<>
      struct value_setter<void>
    {
      // By not taking any arguments, in contrast to `U...` for the
      // default case, this implicitly forbids anything except for
      // `set_value()`, so that an explicit check or specialization
      // for `promise<void>` is not needed.
      template<typename Value>
        void operator() (Value&)
      {}
    };

    // All functions below are effectively members of
    // `promise_future_shared_state`, but that would add a useless
    // level of indirection only, so they are inlined into the
    // respective only callers.

    template<typename T> template<typename... U>
      void promise<T>::set_value (U&&... x)
    {
      std::lock_guard<std::mutex> const _ (_state->_guard);

      if (_state->_content != detail::content::not_yet_set)
      {
        throw util::cxx17::make_future_error
          (std::future_errc::promise_already_satisfied);
      }

      value_setter<T>{} (_state->_value, std::forward<U> (x)...);
      _state->_content = detail::content::value;
      _state->wake();
    }

    template<typename T>
      void promise<T>::set_exception (std::exception_ptr ex)
    {
      std::lock_guard<std::mutex> const _ (_state->_guard);

      if (_state->_content != detail::content::not_yet_set)
      {
        throw util::cxx17::make_future_error
          (std::future_errc::promise_already_satisfied);
      }

      _state->_exception = ex;
      _state->_content = detail::content::exception;
      _state->wake();
    }

    template<typename T>
      future<T> promise<T>::get_future()
    {
      std::lock_guard<std::mutex> const _ (_state->_guard);
      if (_state->_future_created)
      {
        throw util::cxx17::make_future_error
          (std::future_errc::future_already_retrieved);
      }
      _state->_future_created = true;
      return _state;
    }

    template<typename T>
      future<T>::future (std::shared_ptr<detail::promise_future_shared_state<T>> state)
        : _state (state)
    {}

    template<typename T>
      struct value_getter
    {
      // \todo This implies a copy! `std::future` is only get-able
      // once due to moving out, so this should have the same
      // behavior. In addition to moving the value out of the state,
      // drop the reference to `_state` and throw `no_state` on a
      // second `get()`.
      template<typename Value>
        T operator() (Value const& v)
      {
        return v.get();
      }
    };
    template<>
      struct value_getter<void>
    {
      template<typename Value>
        void operator() (Value const&)
      {}
    };

    template<typename T>
      T future<T>::get()
    {
      std::unique_lock<std::mutex> lock (_state->_guard);

      if (_state->_content == detail::content::not_yet_set)
      {
        lock.unlock();
        FHG_UTIL_FINALLY ([&] { lock.lock(); });

        _state->wait();
      }

      if (_state->_content == detail::content::exception)
      {
        std::rethrow_exception (_state->_exception);
      }
      else
      {
        return value_getter<T>{} (_state->_value);
      }
    }
    template<typename T>
      T future<T>::get (::boost::asio::yield_context yield)
    {
      std::unique_lock<std::mutex> lock (_state->_guard);

      if (_state->_content == detail::content::not_yet_set)
      {
        lock.unlock();
        FHG_UTIL_FINALLY ([&] { lock.lock(); });

        _state->wait (yield);
      }

      if (_state->_content == detail::content::exception)
      {
        std::rethrow_exception (_state->_exception);
      }
      else
      {
        return value_getter<T>{} (_state->_value);
      }
    }
  }
}
