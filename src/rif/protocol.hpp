// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <fhglog/level.hpp>
#include <rpc/function_description.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>
#include <util-generic/serialization/std/chrono.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <boost/serialization/optional.hpp>

#include <chrono>
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

      FHG_RPC_FUNCTION_DESCRIPTION
        ( start_vmem
        , pid_t ( boost::filesystem::path command
                , fhg::log::Level log_level
                , std::size_t memory_in_bytes
                , boost::filesystem::path socket
                , unsigned short gaspi_port
                , std::chrono::seconds proc_init_timeout
                , std::string vmem_implementation
                , boost::optional<std::pair<std::string, unsigned short>> log_server
                , boost::optional<boost::filesystem::path> log_file
                , std::vector<std::string> nodes
                , std::string gaspi_master
                , bool is_master
                )
        );
    }
  }
}
