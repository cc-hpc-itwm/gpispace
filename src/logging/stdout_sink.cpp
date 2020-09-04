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

#include <logging/stdout_sink.hpp>

#include <logging/message.hpp>

#include <util-generic/ostream/put_time.hpp>

#include <iostream>

namespace fhg
{
  namespace logging
  {
    stdout_sink::stdout_sink()
      : stdout_sink (std::vector<endpoint>())
    {}
    stdout_sink::stdout_sink (std::vector<endpoint> emitters)
      : stream_receiver
          ( std::move (emitters)
          , [] (fhg::logging::message const& message)
            {
              std::cout << message._process_id << "." << message._thread_id
                        << "@" << message._hostname << " "
                        << util::ostream::put_time<std::chrono::system_clock>
                             (message._timestamp)
                        << " " << message._category
                        << " " << message._content << "\n";
            }
          )
    {}
  }
}
