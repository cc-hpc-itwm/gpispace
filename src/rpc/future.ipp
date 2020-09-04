// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <util-generic/cxx17/future_error.hpp>
#include <util-generic/finally.hpp>

#include <boost/asio/local/connect_pair.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <stdexcept>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      template<typename T>
        promise_future_shared_state<T>::promise_future_shared_state (boost::asio::io_service& io_service)
        : _sender (io_service)
        , _receiver (io_service)
      {
        boost::asio::local::connect_pair (_sender, _receiver);
      }

      template<typename T>
        void promise_future_shared_state<T>::wake()
      {
        char data[1] = {0};
        boost::asio::write (_sender, boost::asio::buffer (data));
      }
      template<typename T>
        void promise_future_shared_state<T>::wait()
      {
        char data[1] = {0};
        boost::asio::read (_receiver, boost::asio::buffer (data));
      }
      template<typename T>
        void promise_future_shared_state<T>::wait (boost::asio::yield_context yield)
      {
        char data[1] = {0};
        boost::asio::async_read
          (_receiver, boost::asio::buffer (data), yield);
      }
    }

    template<typename T>
      promise<T>::promise (boost::asio::io_service& io_service)
        : _state ( std::make_shared<detail::promise_future_shared_state<T>>
                     (io_service)
                 )
    {}

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
      template<typename Value>
        void operator() (Value&)
      {}
    };

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
      T future<T>::get (boost::asio::yield_context yield)
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
