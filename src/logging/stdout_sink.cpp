// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
