// Copyright (C) 2022-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/scheduler/id_generator.hpp>

#include <gspc/util/hostname.hpp>
#include <gspc/util/syscall.hpp>

#include <fmt/core.h>

namespace gspc::scheduler
{
  std::string id_generator::next()
  {
    return _prefix + std::to_string (_counter.fetch_add (1));
  }

  id_generator::id_generator (std::string const& name)
    : _counter (0)
    , _prefix { fmt::format
                ( "{}.{}.{}."
                , gspc::util::hostname()
                , name
                , gspc::util::syscall::getpid()
                )
              }
  {}
}
