// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <fhglog/level.hpp>
#include <rpc/function_description.hpp>

#include <drts/drts.fwd.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>
#include <util-generic/serialization/exception.hpp>
#include <util-generic/serialization/std/chrono.hpp>

#include <logging/endpoint.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

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
        ( start_vmem
        , pid_t ( boost::filesystem::path command
                , fhg::log::Level log_level
                , boost::filesystem::path socket
                , unsigned short gaspi_port
                , std::chrono::seconds proc_init_timeout
                , boost::optional<std::pair<std::string, unsigned short>> log_server
                , boost::optional<boost::filesystem::path> log_file
                , std::vector<std::string> nodes
                , std::string gaspi_master
                , std::size_t rank
                )
        );

      using hostinfo_t = std::pair<std::string, unsigned short>;
      struct start_agent_result
      {
        pid_t pid;
        hostinfo_t hostinfo;
        fhg::logging::endpoint logger_registration_endpoint;
      };

      FHG_RPC_FUNCTION_DESCRIPTION
        ( start_agent
        , start_agent_result
            ( std::string name
            , hostinfo_t parent
            , boost::optional<std::string> gui_host
            , boost::optional<unsigned short> gui_port
            , boost::optional<boost::filesystem::path> gpi_socket
            , gspc::Certificates
            , boost::filesystem::path command
            , std::unordered_map<std::string, std::string> environment
            )
        );

      struct start_worker_result
      {
        pid_t pid;
        fhg::logging::endpoint logger_registration_endpoint;
      };

      FHG_RPC_FUNCTION_DESCRIPTION
        ( start_worker
        , start_worker_result
            ( std::string name
            , boost::filesystem::path command
            , std::vector<std::string> arguments
            , std::unordered_map<std::string, std::string> environment
            )
        );

      struct start_logging_demultiplexer_result
      {
        pid_t pid;
        fhg::logging::endpoint sink_endpoint;
      };

      FHG_RPC_FUNCTION_DESCRIPTION
        ( start_logging_demultiplexer
        , start_logging_demultiplexer_result (boost::filesystem::path exe)
        );
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

    template<typename Archive>
      inline void serialize
        ( Archive& ar
        , fhg::rif::protocol::start_agent_result& result
        , unsigned int
        )
    {
      ar & result.pid;
      ar & result.hostinfo;
      ar & result.logger_registration_endpoint;
    }

    template<typename Archive>
      inline void serialize
        ( Archive& ar
        , fhg::rif::protocol::start_worker_result& result
        , unsigned int
        )
    {
      ar & result.pid;
      ar & result.logger_registration_endpoint;
    }

    template<typename Archive>
      inline void serialize
        ( Archive& ar
        , fhg::rif::protocol::start_logging_demultiplexer_result& result
        , unsigned int
        )
    {
      ar & result.pid;
      ar & result.sink_endpoint;
    }
  }
}
