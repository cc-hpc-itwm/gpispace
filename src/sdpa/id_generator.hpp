// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <util-generic/hostname.hpp>
#include <util-generic/syscall.hpp>

#include <boost/format.hpp>

#include <atomic>

namespace sdpa {
  class id_generator
  {
  public:
    std::string next()
    {
      return _prefix + std::to_string (_counter.fetch_add (1));
    }

    id_generator (std::string const& name)
      : _counter()
      , _prefix ( ( ::boost::format ("%1%.%2%.%3%.")
                  % fhg::util::hostname()
                  % name
                  % fhg::util::syscall::getpid()
                  ).str()
                )
    {}

  private:
    std::atomic<std::size_t> _counter;
    std::string _prefix;
  };
}
