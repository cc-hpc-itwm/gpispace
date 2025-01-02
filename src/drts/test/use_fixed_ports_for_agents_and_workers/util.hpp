// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/finally.hpp>
#include <util-generic/syscall.hpp>

#include <fmt/core.h>
#include <string>

namespace
{
  bool is_using_port (std::string const& component, unsigned int port)
  {
    unsigned int const size (30);
    char data[size] = {0};

    FILE* pf
      ( fhg::util::syscall::popen
          ( fmt::format
            ( "netstat -lntp | grep -w '{}' | awk '{{print $NF}}' | cut -d'/' -f2"
            , port
            ).c_str()
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
