// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-rpc/common.hpp>

#include <util-generic/serialization/exception.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/asio/io_service.hpp>

#include <exception>

namespace fhg
{
  namespace rpc
  {
    //! Base class for the client side of an RPC connection. For use
    //! in interfaces to avoid hard-coding TCP or socket endpoints.
    struct remote_endpoint
    {
      remote_endpoint (util::serialization::exception::serialization_functions functions)
        : _serialization_functions (error::add_builtin (std::move (functions)))
      {}

      remote_endpoint (remote_endpoint const&) = default;
      remote_endpoint (remote_endpoint&&) = default;
      remote_endpoint& operator= (remote_endpoint const&) = default;
      remote_endpoint& operator= (remote_endpoint&&) = default;
      virtual ~remote_endpoint() = default;

      //! Called by \c remote_function to implement the protocol
      //! specific communication. Implementations shall be
      //! non-blocking. Implementations shall call exactly once one of
      //! the two given callbacks once the asynchronous communication
      //! has yielded a result.
      //! \a buffer shall contain the already-serialized data. \a
      //! set_value receives the not-yet-fully-deserialized archive at
      //! the point where user data starts (to avoid a copy). \a
      //! set_exception receives either a deserialized exception or a
      //! locally generated exception (e.g. in case of disconnect).
      virtual void send_and_receive
        ( std::vector<char> buffer
        , std::function<void (::boost::archive::binary_iarchive&)> set_value
        , std::function<void (std::exception_ptr)> set_exception
        ) = 0;
      //! Return the underlying \c io_service. Required to yield when
      //! waiting for completion of a \c remote_function.
      virtual ::boost::asio::io_service& io_service() = 0;

    private:
      template<typename, template<typename> class>
        friend struct remote_function;
      util::serialization::exception::serialization_functions
        _serialization_functions;
    };
  }
}
