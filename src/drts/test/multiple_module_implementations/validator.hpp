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

#pragma once

#include <boost/format.hpp>

#include <stdexcept>
#include <string>

namespace
{
  void validate_implementation
    ( drts::worker::context const* context
    , std::string const& expected_implementation
    )
  {
    auto const workers (context->workers());
    if (workers.size() != 1)
    {
      throw std::runtime_error
        ( ( boost::format ("Unexpected number of workers: %1%!")
          % workers.size()
          ).str()
        );
    }

    auto const worker (context->worker_name());
    if (worker.find (expected_implementation) == std::string::npos)
    {
      throw std::runtime_error
        ( ( boost::format ("Worker %1%: missing capability %2%!")
          % worker
          % expected_implementation
          ).str()
        );
    }
  }
}
