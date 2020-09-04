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

#include <logging/demultiplexer.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/this_bound_mem_fn.hpp>

namespace fhg
{
  namespace logging
  {
    demultiplexer::demultiplexer
        (rif::started_process_promise& promise, int, char**)
      : receiver ( [&] (boost::asio::yield_context yield, message const& m)
                   {
                     return emitter.emit_message (m, std::move (yield));
                   }
                 )
      , add_emitters
          ( service_dispatcher
          , util::bind_this (&receiver, &stream_receiver::add_emitters)
          , fhg::rpc::yielding
          )
      , add_service_provider (io_service, service_dispatcher)
    {
      promise.set_result
        ( emitter.local_endpoint().to_string()
        , socket_endpoint (add_service_provider.local_endpoint()).to_string()
        );
    }
  }
}
