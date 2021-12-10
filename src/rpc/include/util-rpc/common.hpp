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

#include <util-generic/hash/boost/asio/ip/tcp/endpoint.hpp>
#include <util-generic/serialization/boost/asio/ip/tcp/endpoint.hpp>
#include <util-generic/serialization/exception.hpp>

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

namespace fhg
{
  //! The \c rpc ("remote procedure call") namespace provides a
  //! framework for calling functions over process boundaries as if
  //! they were local functions, with automatic serialization of
  //! arguments and return values as well as C++ exceptions.
  //!
  //! RPC functions are declared with \c FHG_RPC_FUNCTION_DESCRIPTION()
  //! and defined with a \c service_handler. Functions have a name and
  //! a signature. Functions are called with \c remote_function. Both
  //! \c service_handler and \c remote_function take the name as
  //! template parameter to avoid typos off the name and check
  //! signatures of handlers and calls at compile time.
  //!
  //! The underlying transport layers use Boost.Asio so both service
  //! provider and caller need to have a \c ::boost::asio::io_service
  //! ready with threads running (there are provider variants that
  //! allow for forking and deferred starting). A caller usually needs
  //! just one thread unless it is doing a lot of calls in parallel:
  //! Calls are not blocking. On the provider side handlers of calls
  //! *are* blocking, so multiple threads may help. Util-Generic
  //! provides \c util::scoped_boost_asio_io_service_with_threads.
  //!
  //! On the provider side, to allow for multiple transport layers
  //! being used at the same time, \c service_handler are registered
  //! in a \c service_dispatcher. This dispatcher is given to one or
  //! more \c service_tcp_provider or \c service_socket_provider.
  //! These listen either on a given endpoint or automatically
  //! determine an endpoint that can be queried using `.local_endpoint()`.
  //!
  //! In the caller process, a corresponding \c remote_tcp_endpoint or
  //! \c remote_socket_endpoint is created to connect to a
  //! provider. Using this endpoint, a \c remote_function functor can
  //! be created and called. It returns a \c std::future which is used
  //! to wait for asynchronous completion of the function call.
  //!
  //! Since waiting on a future is still blocking a thread and thus
  //! can't be used in a cooperative coroutine, a \c future exists
  //! which supports yielding to Boost.Asio. It can be used by \c
  //! remote_function to allow for recursive calls to other RPC
  //! providers from within a \c service_handler.
  //!
  //! The \c locked_with_info_file helpers wrap the pattern of a
  //! unique server that's communicating connection information via a
  //! shared filesystem.
  //!
  //! In summary, usually:
  //!
  //! - A shared header has \c FHG_RPC_FUNCTION_DESCRIPTION() calls to
  //!  define the API.
  //!
  //!- A caller has
  //!  - a \c ::boost::asio::io_service with threads
  //!  - usually one \c remote_xxx_endpoint per service provider used
  //!  - usually one \c remote_function per API function called
  //!  - may have a `client` convenience wrapper around a collection
  //!    of \c remote_function for a single `remote_xxx_endpoint` to
  //!    treat the remote call like a method of an object representing
  //!    the remote
  //!
  //!- A provider has
  //!  - an \c ::boost::asio::io_service with threads
  //!  - usually one \c service_dispatcher
  //!  - one \c service_handler per API function provided
  //!  - one or more `service_xxx_provider` per transport layer
  //!    provided
  namespace rpc
  {
    namespace error
    {
      struct duplicate_function : std::logic_error
      {
        std::string function_name;
        duplicate_function (decltype (function_name) name)
          : std::logic_error ("duplicate function name '" + name + "'")
          , function_name (std::move (name))
        {}

        //! Serialize to a string.
        std::string to_string() const { return function_name; }
        //! Deserialize from a string.
        static duplicate_function from_string (std::string name) { return name; }
      };

      struct unknown_function : std::logic_error
      {
        std::string function_name;
        unknown_function (decltype (function_name) name)
          : std::logic_error  ("function '" + name + "' does not exist")
          , function_name (std::move (name))
        {}

        //! Serialize to a string.
        std::string to_string() const { return function_name; }
        //! Deserialize from a string.
        static unknown_function from_string (std::string name) { return name; }
      };

      using namespace util::serialization;
      //! Extend the set of util-generic-exception-serialization
      //! functions \a functions with the rpc-built-in exception
      //! types.
      inline exception::serialization_functions add_builtin
        (exception::serialization_functions functions)
      {
        functions.emplace (exception::make_functions<duplicate_function>());
        functions.emplace (exception::make_functions<unknown_function>());
        return functions;
      }
    }
  }
}
