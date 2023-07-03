// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sdpa/id_generator.hpp>

#include <util-generic/hostname.hpp>
#include <util-generic/syscall.hpp>

#include <boost/format.hpp>

namespace sdpa
{
  std::string id_generator::next()
  {
    return _prefix + std::to_string (_counter.fetch_add (1));
  }

  id_generator::id_generator (std::string const& name)
    : _counter (0)
    , _prefix ( ( ::boost::format ("%1%.%2%.%3%.")
                % fhg::util::hostname()
                % name
                % fhg::util::syscall::getpid()
                ).str()
              )
  {}
}
