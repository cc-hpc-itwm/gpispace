// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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
