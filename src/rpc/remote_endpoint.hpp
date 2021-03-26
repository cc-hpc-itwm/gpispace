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

#include <rpc/common.hpp>

#include <util-generic/serialization/exception.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/asio/io_service.hpp>

#include <exception>

namespace fhg
{
  namespace rpc
  {
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

      virtual void send_and_receive
        ( std::vector<char> buffer
        , std::function<void (boost::archive::binary_iarchive&)> set_value
        , std::function<void (std::exception_ptr)> set_exception
        ) = 0;
      virtual boost::asio::io_service& io_service() = 0;

    private:
      template<typename, template<typename> class>
        friend struct remote_function;
      util::serialization::exception::serialization_functions
        _serialization_functions;
    };
  }
}
