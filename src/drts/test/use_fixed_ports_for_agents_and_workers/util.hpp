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

#include <util-generic/finally.hpp>
#include <util-generic/syscall.hpp>

#include <boost/format.hpp>

#include <string>

namespace
{
  bool is_using_port (std::string const& component, unsigned int port)
  {
    unsigned int const size (30);
    char data[size] = {0};

    FILE* pf
      ( fhg::util::syscall::popen
          ( ( boost::format
               ("netstat -lntp | grep -w '%1%' | awk '{print $NF}' | cut -d'/' -f2")
            % port
            ).str().c_str()
          , "r"
          )
      );

    FHG_UTIL_FINALLY ([&] { fhg::util::syscall::pclose (pf); });
    while (fhg::util::syscall::fgets (data, size, pf))
    {
      std::string process (data);
      process.erase (process.find_last_not_of ("\n") + 1);

      if (component == process)
      {
        return true;
      }
    }

    return false;
  }
}
