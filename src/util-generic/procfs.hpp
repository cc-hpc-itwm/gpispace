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

#include <boost/filesystem/path.hpp>

#include <list>
#include <string>

namespace fhg
{
  namespace util
  {
    //! \note races everywhere
    namespace procfs
    {
      struct entry
      {
        entry (pid_t);
        entry (std::string const& pid)
          : entry (std::stoi (pid))
        {}

        pid_t pid() const
        {
          return _pid;
        }
        std::list<std::string> const& command_line() const
        {
          return _command_line;
        }

      private:
        pid_t const _pid;
        std::list<std::string> const _command_line;
      };

      std::list<procfs::entry> entries();
    }
  }
}
