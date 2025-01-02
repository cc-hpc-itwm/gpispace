// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/exit_status.hpp>
#include <util-generic/system.hpp>

#include <fmt/core.h>
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
          { fmt::format ( "Could not execute '{}': {}"
                        , command
                        , std::strerror (wexitstatus (ec))
                        )
          };
      }

      return;
    }
  }
}
