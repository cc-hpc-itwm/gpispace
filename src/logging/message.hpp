// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <logging/legacy/event.hpp>

#include <chrono>
#include <string>

#include <sys/types.h>

namespace fhg
{
  namespace logging
  {
    struct message
    {
      std::string _content;
      std::string _category;

      std::chrono::system_clock::time_point _timestamp;
      std::string _hostname;
      pid_t _process_id;
      pid_t _thread_id;

      message (decltype (_content), decltype (_category));

      message() = default;
      template<typename Archive>
        void serialize (Archive& ar, unsigned int);

    private:
      message ( decltype (_content)
              , decltype (_category)
              , decltype (_timestamp)
              , decltype (_hostname)
              , decltype (_process_id)
              , decltype (_thread_id)
              );
    };
  }
}

#include <logging/message.ipp>
