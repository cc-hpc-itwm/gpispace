// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/filesystem/path.hpp>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fhg
{
  namespace iml
  {
    namespace rif
    {
      //! \note child shall not exit with >= 240: used for execve-error handling
      //! \note child shall use rif::startup_messages_pipe after
      //!       initializing and let it go out of scope at the time of
      //!       wanting to be demonized. exiting after closing pipe is
      //!       not an error
      std::pair<pid_t, std::vector<std::string>> execute_and_get_startup_messages
        ( ::boost::filesystem::path command
        , std::vector<std::string> arguments
        , std::unordered_map<std::string, std::string> environment
        );
    }
  }
}