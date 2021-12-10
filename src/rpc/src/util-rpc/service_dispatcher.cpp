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

#include <util-rpc/service_dispatcher.hpp>

#include <util-rpc/common.hpp>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>

#include <stdexcept>

namespace fhg
{
  namespace rpc
  {
    service_dispatcher::service_dispatcher
        (util::serialization::exception::serialization_functions functions)
          : _serialization_functions (error::add_builtin (std::move (functions)))
    {}

    void service_dispatcher::dispatch ( ::boost::asio::yield_context yield
                                      , ::boost::archive::binary_iarchive& input
                                      , ::boost::archive::binary_oarchive& output
                                      ) const
    {
      std::string function;
      input >> function;

      try
      {
        decltype (_handlers)::const_iterator const handler
          (_handlers.find (function));

        if (handler == _handlers.end())
        {
          throw error::unknown_function (function);
        }

        handler->second (yield, input, output);
      }
      catch (...)
      {
        output << true;
        util::serialization::exception::serialize
          (output, std::current_exception(), _serialization_functions);
      }
    }
  }
}
