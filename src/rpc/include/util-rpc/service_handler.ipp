// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/callable_signature.hpp>
#include <util-generic/serialization/std/tuple.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <tuple>

namespace fhg
{
  namespace rpc
  {
    template<typename> struct service_handler_thunk;

    template<typename Description> template<typename Func, typename Yielding>
      service_handler<Description>::service_handler
          (service_dispatcher& dispatcher, Func&& handler, Yielding)
        : _handler_registration
            ( dispatcher._handlers
            , typeid (Description).name()
            , [handler] ( ::boost::asio::yield_context yield
                        , ::boost::archive::binary_iarchive& input
                        , ::boost::archive::binary_oarchive& output
                        )
              {
                service_handler_thunk<typename Description::signature>{}
                  (yield, handler, input, output, Yielding{});
              }
            )
    {}

    template<typename... Args>
      std::tuple<Args...> arguments ( ::boost::archive::binary_iarchive& input
                                    , ::boost::asio::yield_context
                                    , decltype (not_yielding)
                                    )
    {
      std::tuple<Args...> arguments;
      input >> arguments;
      return arguments;
    }
    template<typename... Args>
      std::tuple<::boost::asio::yield_context, Args...>
        arguments ( ::boost::archive::binary_iarchive& input
                  , ::boost::asio::yield_context yield
                  , decltype (yielding)
                  )
    {
      return std::tuple_cat
        ( std::forward_as_tuple (yield)
        , arguments<Args...> (input, yield, not_yielding)
        );
    }


    template<typename R, typename... Args>
      struct service_handler_thunk<R (Args...)>
    {
    public:
      template<typename Func, typename Yielding>
        void operator() ( ::boost::asio::yield_context yield
                        , Func&& fun
                        , ::boost::archive::binary_iarchive& input
                        , ::boost::archive::binary_oarchive& output
                        , Yielding
                        )
      {
        R result ( std::apply
                     (fun, arguments<Args...> (input, yield, Yielding{}))
                 );
        output << false;
        output << result;
      }
    };

    template<typename... Args>
      struct service_handler_thunk<void (Args...)>
    {
    public:
      template<typename Func, typename Yielding>
        void operator() ( ::boost::asio::yield_context yield
                        , Func&& fun
                        , ::boost::archive::binary_iarchive& input
                        , ::boost::archive::binary_oarchive& output
                        , Yielding
                        )
      {
        std::apply (fun, arguments<Args...> (input, yield, Yielding{}));
        output << false;
      }
    };
  }
}
