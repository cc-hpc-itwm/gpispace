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

#include <util-rpc/detail/vector_sink.hpp>

#include <util-generic/callable_signature.hpp>
#include <util-generic/serialization/std/tuple.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/vector.hpp>

#include <sstream>
#include <string>

namespace fhg
{
  namespace rpc
  {
    template<typename T>
      struct deserialize_from_buffer
    {
      template<typename Promise>
        inline void operator()
          (Promise& promise, ::boost::archive::binary_iarchive& archive)
      {
        T value;
        archive >> value;
        promise.set_value (std::move (value));
      }
    };
    template<>
      struct deserialize_from_buffer<void>
    {
      template<typename Promise>
        inline void operator()
          (Promise& promise, ::boost::archive::binary_iarchive&)
      {
        promise.set_value();
      }
    };

    // Allow always passing `io_service` in generic code, but don't
    // actually pass it to `std::promise`.
    template<typename Future>
      struct promise_for;
    template<typename T>
      struct promise_for<std::future<T>>
    {
      using type = std::promise<T>;
      static std::shared_ptr<type> make_shared (::boost::asio::io_service&)
      {
        return std::make_shared<type>();
      }
    };
    template<typename T>
      struct promise_for<future<T>>
    {
      using type = promise<T>;
      static std::shared_ptr<type> make_shared (::boost::asio::io_service& io_service)
      {
        return std::make_shared<type> (io_service);
      }
    };

    template<typename Description, template<typename> class Future>
      remote_function<Description, Future>::remote_function
          (remote_endpoint& endpoint)
        : _endpoint (endpoint)
    {
      //! \todo check that function with signature exists, optionally?
    }

    namespace
    {
      template<typename T, typename U> struct try_forward { using type = T; };
      template<typename T> struct try_forward<T, T> { using type = T&&; };

      template<typename, typename...> struct arguments_tuple_of;
      template<typename R, typename... Args, typename... Brgs>
        struct arguments_tuple_of<R (Args...), Brgs...>
      {
        using type = std::tuple<typename try_forward<Args, Brgs>::type...>;
      };

      template<typename... T>
        using arguments_tuple_of_t = typename arguments_tuple_of<T...>::type;

      static_assert ( std::is_same < arguments_tuple_of_t < void (char, char)
                                                          , char, char
                                                          >
                                   , std::tuple<char&&, char&&>
                                   >::value
                    , "shall forward for matching types"
                    );
      static_assert ( std::is_same < arguments_tuple_of_t < void (int, int)
                                                          , char, char
                                                          >
                                   , std::tuple<int, int>
                                   >::value
                    , "shall copy for non matching types"
                    );
      static_assert ( std::is_same < arguments_tuple_of_t < void (int, char)
                                                          , char, char
                                                          >
                                   , std::tuple<int, char&&>
                                   >::value
                    , "shall work for interleaved sets"
                    );
    }

    template<typename Description, template<typename> class Future>
      template<typename... Args>
        auto remote_function<Description, Future>::operator() (Args&&... args)
          -> Future<typename Description::result_type>
    {
      static_assert ( util::is_callable
                        < typename Description::signature
                        , typename Description::result_type (Args&&...)
                        >::value
                    , "Args... do not match signature"
                    );

      std::vector<char> request;
      {
        detail::vector_sink sink (request);
        ::boost::iostreams::stream<decltype (sink)> stream (sink);
        ::boost::archive::binary_oarchive archive (stream);
        archive << std::string (typeid (Description).name());
        archive << arguments_tuple_of_t < typename Description::signature
                                        , Args...
                                        > (std::forward<Args> (args)...);
      }

      //! \note maximum use_count: 3 during construction, 2 while
      //! waiting. Can't be class member as remote_function might be a
      //! temporary only used to return the future and the future
      //! being waited for.
      auto promise
        ( promise_for<Future<typename Description::result_type>>::make_shared
            (_endpoint.io_service())
        );

      auto* endpoint (&_endpoint);
      _endpoint.send_and_receive
        ( std::move (request)
        , [promise, endpoint] (::boost::archive::binary_iarchive& archive)
          {
            bool is_exception;
            archive >> is_exception;

            if (is_exception)
            {
              promise->set_exception
                ( util::serialization::exception::deserialize
                    (archive, endpoint->_serialization_functions)
                );
            }
            else
            {
              try
              {
                deserialize_from_buffer<typename Description::result_type>{}
                  (*promise, archive);
              }
              catch (...)
              {
                promise->set_exception (std::current_exception());
              }
            }
          }
        , [promise] (std::exception_ptr exception)
          {
            promise->set_exception (exception);
          }
        );

      return promise->get_future();
    }

    template<typename Description, template<typename> class Future>
      sync_remote_function<Description, Future>::sync_remote_function
          (remote_endpoint& endpoint)
        : _function (endpoint)
    {}

    namespace detail
    {
#define RETURNS(...)                                                    \
      noexcept (noexcept (__VA_ARGS__)) -> decltype (__VA_ARGS__)       \
      {                                                                 \
        return __VA_ARGS__;                                             \
      }

      template<typename Fun, typename... Args>
        auto sync (Fun&& fun, Args&&... args)
          RETURNS (fun (std::forward<Args> (args)...).get())

      template<typename Fun, typename... Args>
        auto sync (Fun&& fun, ::boost::asio::yield_context yield, Args&&... args)
          RETURNS (fun (std::forward<Args> (args)...).get (std::move (yield)))

#undef RETURNS
    }

    template<typename Description, template<typename> class Future>
      template<typename... Args>
        auto sync_remote_function<Description, Future>::operator() (Args&&... args)
          -> typename Description::result_type
    {
      // Split `yield` from remaining arguments and pass to `.get()`
      // instead: either `_function (args...).get()` or `_function
      // (args...).get (yield)`.
      return detail::sync (_function, std::forward<Args> (args)...);
    }
  }
}
