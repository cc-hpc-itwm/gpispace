// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-rpc/service_dispatcher.hpp>

#include <util-rpc/common.hpp>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

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
