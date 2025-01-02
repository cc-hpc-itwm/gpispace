// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-rpc/common.hpp>
#include <util-rpc/function_description.hpp>
#include <util-rpc/future.hpp>
#include <util-rpc/remote_endpoint.hpp>

#include <future>

namespace fhg
{
  namespace rpc
  {
    //! Functor to call the remote function \a Description which has
    //! been declared using \c FHG_RPC_FUNCTION_DESCRIPTION. Calls are
    //! made asynchronously, which is why `operator()` returns a \a
    //! Future instead of the result directly.
    //!
    //! When calling a remote function from a service handler \a
    //! Future can be specified to be \c rpc::future, which allows
    //! passing a \c yield context to \c rpc::future::get().
    //!
    //! \see sync_remote_function to avoid `.get()` calls if calls are
    //! always synchronous.
    //!
    //! \note \a Future defaults to \c std::future instead of \c
    //! rpc::future to avoid resource overhead when yielding is not
    //! required.
    template < typename Description
             , template<typename> class Future = std::future
             >
      struct remote_function
    {
    private:
      static_assert ( is_function_description_t<Description>::value
                    , "Description shall be a FHG_RPC_FUNCTION_DESCRIPTION"
                    );

    public:
      //! Construct a functor to call the function on \a endpoint.
      //! \note No check of existence happens at this point. If the
      //! function does not, `operator()` will throw instead.
      remote_function (remote_endpoint& endpoint);

      //! Call the represented function with the arguments given,
      //! asynchronously.
      //!
      //! The result can be retrieved from the returned \c Future via
      //! `.get()` which either returns the specific return value or
      //! an exception that's thrown from many possible sources:
      //! - by the remote service user implementation
      //! - by the remote service dispatcher (e.g. "function not
      //!   found")
      //! - by the remote service network implementation
      //!   (e.g. out-of-resources)
      //! - by the local network implementation (e.g. disconnected)
      //! - by the serialization implementation of return value or
      //!   exceptions
      //! Additionally, `operator()` may throw itself, e.g.
      //! - by the serialization implementation of arguments
      //!
      //! \note The returned future may outlive the \c remote_function
      //! and still end successfully. Once the corresponding \c
      //! remote_endpoint is destroyed though, the future will contain
      //! an exception.
      //!
      //! \note While exceptions will be transferred from remote, note
      //! that it requires custom exception types to be registered in
      //! the remote_endpoint and service_dispatcher. Else, exceptions
      //! will be down-cast to the nearest std::* exception class; or
      //! service_dispatcher will std::terminate() when the exception
      //! does not inherit from a std::* exception class. Note that if
      //! an exception serialization is registered in
      //! service_dispatcher, it needs to be registered in
      //! remote_endpoint as well to properly deserialize and avoid an
      //! std::terminate() it does not inherit from a std::* exception
      //! class. If it does, it will be downcasted automatically. On
      //! the API to implement, see `src/rpc/exception_serialization.hpp`.
      template<typename... Args>
        Future<typename Description::result_type> operator() (Args&&...);

    private:
      remote_endpoint& _endpoint;
    };

    //! Wrapper for \c remote_function<Description,Future> which
    //! automatically calls `.get()` on the asynchronous call result.
    template < typename Description
             , template<typename> class Future = std::future
             >
      struct sync_remote_function
    {
    private:
      static_assert ( is_function_description_t<Description>::value
                    , "Description shall be a FHG_RPC_FUNCTION_DESCRIPTION"
                    );

    public:
      //! Identical to \c remote_function::remote_function().
      sync_remote_function (remote_endpoint&);

      //! Identical to \c remote_function::operator() except that
      //! instead of returning a \c Future, it returns the result of
      //! \c Future::get().
      //!
      //! When using a \c rpc::future, pass the yield context as first
      //! argument.
      //!
      //! Given `FHG_RPC_FUNCTION_DESCRIPTION (sum, int (int, int))`
      //! the following to pairs are semantically identical:
      //! - `sync_remote_function<sum> {client} (x, y)`
      //!   `remote_function<sum> {client} (x, y).get()`
      //! - `sync_remote_function<sum, rpc::future> {client} (x, y)`
      //!   `remote_function<sum, rpc::future> {client} (x, y).get()`
      //! - `sync_remote_function<sum, rpc::future> {client} (yield, x, y)`
      //!   `remote_function<sum, rpc::future> {client} (x, y).get (yield)`
      template<typename... Args>
        typename Description::result_type operator() (Args&&...);

    private:
      remote_function<Description, Future> _function;
    };
  }
}

#include <util-rpc/remote_function.ipp>
