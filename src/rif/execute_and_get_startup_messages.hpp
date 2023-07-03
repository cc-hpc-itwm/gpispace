// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
