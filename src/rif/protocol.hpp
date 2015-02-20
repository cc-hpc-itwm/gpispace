// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <rpc/function_description.hpp>

#include <boost/filesystem/path.hpp>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <unistd.h>

namespace fhg
{
  namespace rif
  {
    namespace protocol
    {
      FHG_RPC_FUNCTION_DESCRIPTION
        ( execute_and_get_startup_messages
        , std::pair<pid_t, std::vector<std::string>>
            ( boost::filesystem::path command
            , std::vector<std::string> arguments
            , std::unordered_map<std::string, std::string> environment
            )
        );

      FHG_RPC_FUNCTION_DESCRIPTION (kill, void (std::vector<pid_t> pids));
    }
  }
}
