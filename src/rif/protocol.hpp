// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <fhglog/level.hpp>
#include <rpc/function_description.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>
#include <util-generic/serialization/exception.hpp>
#include <util-generic/serialization/std/chrono.hpp>

#include <vmem/server/server_communication.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/utility.hpp>

#include <chrono>
#include <exception>
#include <string>
#include <unordered_map>
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

      FHG_RPC_FUNCTION_DESCRIPTION
        ( execute_and_get_startup_messages_and_wait
        , std::vector<std::string>
            ( boost::filesystem::path command
            , std::vector<std::string> arguments
            , std::unordered_map<std::string, std::string> environment
            )
        );

      FHG_RPC_FUNCTION_DESCRIPTION
        ( kill
        , std::unordered_map<pid_t, std::exception_ptr> (std::vector<pid_t>)
        );

      FHG_RPC_FUNCTION_DESCRIPTION
        ( start_vmem_step_a
        , std::pair<pid_t, std::uint16_t>
            ( boost::filesystem::path command
            , boost::filesystem::path socket
            , unsigned short gaspi_port
            , std::chrono::seconds proc_init_timeout
            )
        );

      FHG_RPC_FUNCTION_DESCRIPTION
        ( start_vmem_step_b
        , void ( pid_t
               , std::vector<intertwine::vmem::node> nodes
               , uint16_t local_communication_port
               )
        );

      namespace local
      {
        FHG_RPC_FUNCTION_DESCRIPTION
          ( vmem_set_port_and_continue
          , void ( std::vector<intertwine::vmem::node> nodes
                 , uint16_t local_communication_port
                 )
          );
        FHG_RPC_FUNCTION_DESCRIPTION (vmem_setup_finished, void());
      }
    }
  }
}

namespace boost
{
  namespace serialization
  {
    template<class Archive>
      inline void save
        ( Archive& ar
        , const std::unordered_map<pid_t, std::exception_ptr>& t
        , const unsigned int
        )
    {
      std::size_t const size (t.size());
      ar << size;
      for (auto const& x : t)
      {
        std::string const exception_string
          (fhg::util::serialization::exception::serialize (x.second));
        ar << x.first;
        ar << exception_string;
      }
    }

    template<class Archive>
      inline void load
        ( Archive& ar
        , std::unordered_map<pid_t, std::exception_ptr>& t
        , const unsigned int
        )
    {
      std::size_t size;
      ar >> size;
      while (size --> 0)
      {
        pid_t pid;
        std::string exception_string;
        ar >> pid;
        ar >> exception_string;
        t.emplace
          ( pid
          , fhg::util::serialization::exception::deserialize (exception_string)
          );
      }
    }

    template<class Archive, class Compare, class Allocator>
      inline void serialize
        ( Archive& ar
        , std::unordered_map<pid_t, std::exception_ptr>& t
        , const unsigned int file_version
        )
    {
      boost::serialization::split_free (ar, t, file_version);
    }
  }
}
