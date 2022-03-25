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

#include <logging/message.hpp>

#include <util-generic/hostname.hpp>
#include <util-generic/syscall.hpp>

#include <stdexcept>

namespace fhg
{
  namespace logging
  {
    message::message ( decltype (_content) content
                     , decltype (_category) category
                     )
      : message ( std::move (content)
                , std::move (category)
                , decltype (_timestamp)::clock::now()
                , util::hostname()
                , util::syscall::getpid()
                , util::syscall::gettid()
                )
    {}
    message::message ( decltype (_content) content
                     , decltype (_category) category
                     , decltype (_timestamp) timestamp
                     , decltype (_hostname) hostname
                     , decltype (_process_id) process_id
                     , decltype (_thread_id) thread_id
                     )
      : _content (std::move (content))
      , _category (std::move (category))
      , _timestamp (std::move (timestamp))
      , _hostname (std::move (hostname))
      , _process_id (std::move (process_id))
      , _thread_id (std::move (thread_id))
    {}
  }
}
