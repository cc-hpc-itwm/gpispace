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
