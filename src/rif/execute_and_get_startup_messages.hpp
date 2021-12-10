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

#include <boost/filesystem/path.hpp>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fhg
{
  namespace rif
  {
    //! \note child shall not exit with >= 240: used for execve-error handling
    //! \note child shall use rif::startup_messages_pipe after
    //!       initializing and let it go out of scope at the time of
    //!       wanting to be demonized. exiting after closing pipe is
    //!       not an error
    struct StartupResult
    {
      pid_t pid;
      std::vector<std::string> messages;
    };
    StartupResult execute_and_get_startup_messages
      ( ::boost::filesystem::path command
      , std::vector<std::string> arguments
      , std::unordered_map<std::string, std::string> environment
      );
  }
}
