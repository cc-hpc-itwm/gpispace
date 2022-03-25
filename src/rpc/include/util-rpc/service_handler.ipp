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

#include <util-generic/callable_signature.hpp>
#include <util-generic/cxx17/apply.hpp>
#include <util-generic/serialization/std/tuple.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

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
        R result ( util::cxx17::apply
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
        util::cxx17::apply (fun, arguments<Args...> (input, yield, Yielding{}));
        output << false;
      }
    };
  }
}
