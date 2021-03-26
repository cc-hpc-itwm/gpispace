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

#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/optional.hpp>

#include <future>
#include <mutex>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      enum class content
      {
        not_yet_set,
        value,
        exception,
      };

      template<typename T>
        struct promise_future_shared_state
      {
        promise_future_shared_state (boost::asio::io_service&);

        void wake();
        void wait();
        void wait (boost::asio::yield_context);

        boost::asio::local::stream_protocol::socket _sender;
        boost::asio::local::stream_protocol::socket _receiver;

        std::mutex _guard;
        content _content = content::not_yet_set;
        std::exception_ptr _exception;
        bool _future_created = false;
        typename std::conditional<std::is_void<T>{}, void*, boost::optional<T>>::type
          _value;
      };
    }

    template<typename T> struct promise;
    template<typename T> struct future;

    template<typename T>
      struct promise
    {
      promise (boost::asio::io_service&);

      template<typename... U>
        void set_value (U&&...);
      void set_exception (std::exception_ptr);

      future<T> get_future();

      promise (promise<T> const&) = delete;
      promise (promise<T>&&) = delete;
      promise& operator= (promise<T> const&) = delete;
      promise& operator= (promise<T>&&) = delete;

    private:
      friend struct future<T>;

      std::shared_ptr<detail::promise_future_shared_state<T>> _state;
    };

    template<typename T>
      struct future
    {
      T get();
      T get (boost::asio::yield_context);

      future (future<T> const&) = delete;
      future (future<T>&&) = default;
      future& operator= (future<T> const&) = delete;
      future& operator= (future<T>&&) = delete;

    private:
      friend struct promise<T>;

      future (std::shared_ptr<detail::promise_future_shared_state<T>>);
      std::shared_ptr<detail::promise_future_shared_state<T>> _state;
    };
  }
}

#include <rpc/future.ipp>
