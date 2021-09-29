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

#include <util-generic/system.hpp>
#include <util-generic/exit_status.hpp>

#include <boost/format.hpp>

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <sys/wait.h>

namespace fhg
{
  namespace util
  {
    template<>
      void system<Command> (Command command)
    {
      if (int ec = std::system (command.c_str()))
      {
        throw std::runtime_error
          ( ( boost::format ("Could not execute '%1%': %2%")
            % command
            % std::strerror (wexitstatus (ec))
            ).str()
          );
      }

      return;
    }
  }
}
