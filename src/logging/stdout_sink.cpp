// Copyright (C) 2019,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/logging/stdout_sink.hpp>

#include <gspc/logging/message.hpp>

#include <gspc/util/ostream/put_time.hpp>

#include <iostream>


  namespace gspc::logging
  {
    stdout_sink::stdout_sink()
      : stdout_sink (std::vector<endpoint>())
    {}
    stdout_sink::stdout_sink (std::vector<endpoint> emitters)
      : stream_receiver
          ( std::move (emitters)
          , [] (gspc::logging::message const& message)
            {
              std::cout << message._process_id << "." << message._thread_id
                        << "@" << message._hostname << " "
                        << gspc::util::ostream::put_time<std::chrono::system_clock>
                             (message._timestamp)
                        << " " << message._category
                        << " " << message._content << "\n";
            }
          )
    {}
  }
