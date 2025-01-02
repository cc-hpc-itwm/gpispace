// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
