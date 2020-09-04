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

#pragma once

#include <util-generic/serialization/exception.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/asio/spawn.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace fhg
{
  namespace rpc
  {
    struct service_dispatcher
    {
    public:
      service_dispatcher ( util::serialization::exception::serialization_functions
                         = util::serialization::exception::serialization_functions()
                         );

      void dispatch ( boost::asio::yield_context
                    , boost::archive::binary_iarchive&
                    , boost::archive::binary_oarchive&
                    ) const;

      service_dispatcher (service_dispatcher const&) = delete;
      service_dispatcher (service_dispatcher&&) = delete;
      service_dispatcher& operator= (service_dispatcher const&) = delete;
      service_dispatcher& operator= (service_dispatcher&&) = delete;

    private:
      template<typename> friend struct service_handler;

      std::unordered_map
        < std::string
        , std::function< void ( boost::asio::yield_context
                              , boost::archive::binary_iarchive&
                              , boost::archive::binary_oarchive&
                              )
                       >
        > _handlers;

      util::serialization::exception::serialization_functions _serialization_functions;
    };
  }
}
