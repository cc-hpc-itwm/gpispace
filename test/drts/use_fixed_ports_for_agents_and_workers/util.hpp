// Copyright (C) 2019-2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/finally.hpp>
#include <gspc/util/syscall.hpp>

#include <fmt/core.h>
#include <string>

namespace
{
  bool is_using_port (std::string const& component, unsigned int port)
  {
    unsigned int const size (30);
    char data[size] = {0};

    FILE* pf
      ( gspc::util::syscall::popen
          ( fmt::format
            ( "netstat -lntp | grep -w '{}' | awk '{{print $NF}}' | cut -d'/' -f2"
            , port
            ).c_str()
          , "r"
          )
      );

    FHG_UTIL_FINALLY ([&] { gspc::util::syscall::pclose (pf); });
    while (gspc::util::syscall::fgets (data, size, pf))
    {
      std::string process (data);
      process.erase (process.find_last_not_of ("\n") + 1);

      // netstat may truncate process names in the PID/program column.
      if ( component == process
        || component.find (process) == 0
        || process.find (component) == 0
         )
      {
        return true;
      }
    }

    return false;
  }
}
