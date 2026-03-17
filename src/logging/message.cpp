// Copyright (C) 2018-2019,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/logging/message.hpp>

#include <gspc/util/hostname.hpp>
#include <gspc/util/syscall.hpp>

#include <stdexcept>


  namespace gspc::logging
  {
    message::message ( decltype (_content) content
                     , decltype (_category) category
                     )
      : message ( std::move (content)
                , std::move (category)
                , decltype (_timestamp)::clock::now()
                , gspc::util::hostname()
                , gspc::util::syscall::getpid()
                , gspc::util::syscall::gettid()
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
