// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <drts/certificates.hpp>
#include <drts/drts.fwd.hpp>
#include <drts/worker_description.hpp>

#include <installation_path.hpp>

#include <fhg/util/signal_handler_manager.hpp>

#include <logging/endpoint.hpp>

#include <rif/entry_point.hpp>
#include <rif/protocol.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <chrono>
#include <cstddef>
#include <exception>
#include <mutex>
#include <optional>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace fhg
{
  namespace drts
  {
    using hostinfo_type = std::pair<std::string, unsigned short>;

    enum class component_type
    {
      agent,
      worker,
      logging_demultiplexer,
    };

    struct processes_storage
    {
      std::mutex _guard;
      std::unordered_map < fhg::rif::entry_point
                         , std::unordered_map<std::string /*name*/, pid_t>
                         > _;

      processes_storage (std::ostream& info_output)
        : _info_output (info_output)
      {}

      ~processes_storage();
      processes_storage (processes_storage const&) = delete;
      processes_storage (processes_storage&&) = delete;
      processes_storage& operator= (processes_storage const&) = delete;
      processes_storage& operator= (processes_storage&&) = delete;
      std::unordered_map
        < rif::entry_point
        , std::pair< std::string /* kind */
                   , std::unordered_map<pid_t, std::exception_ptr>
                   >
        > shutdown_worker (std::vector<fhg::rif::entry_point> const&);

      void store (fhg::rif::entry_point const&, std::string const& name, pid_t);
      std::optional<pid_t> pidof
        (fhg::rif::entry_point const&, std::string const& name);

    private:
      std::ostream& _info_output;
    };

    std::pair< std::unordered_set<fhg::rif::entry_point>
             , std::unordered_map<fhg::rif::entry_point, std::exception_ptr>
             > start_workers_for
      ( std::vector<fhg::rif::entry_point> const& entry_points
      , std::string parent_name
      , fhg::drts::hostinfo_type parent_hostinfo
      , gspc::worker_description const& description
      , fhg::drts::processes_storage& processes
      , ::boost::optional<::boost::filesystem::path> const& gpi_socket
      , std::vector<::boost::filesystem::path> const& app_path
      , std::vector<std::string> const& worker_env_copy_variable
      , bool worker_env_copy_current
      , std::vector<::boost::filesystem::path> const& worker_env_copy_file
      , std::vector<std::string> const& worker_env_set_variable
      , gspc::installation_path const&
      , std::ostream& info_output
      , ::boost::optional<std::pair<fhg::rif::entry_point, pid_t>> top_level_log
      , gspc::Certificates const&
      );

    struct startup_result
    {
      hostinfo_type top_level_agent;
      ::boost::optional<rif::protocol::start_logging_demultiplexer_result>
        top_level_logging_demultiplexer;
    };

    startup_result startup
      ( ::boost::optional<unsigned short> const& agent_port
      , ::boost::optional<::boost::filesystem::path> gpi_socket
      , gspc::installation_path const&
      , fhg::util::signal_handler_manager& signal_handler_manager
      , std::vector<fhg::rif::entry_point> const&
      , fhg::rif::entry_point const&
      , processes_storage&
      , std::string& parent_agent_name
      , fhg::drts::hostinfo_type& parent_agent_hostinfo
      , std::ostream& info_output
      , ::boost::optional<fhg::rif::entry_point> log_rif_entry_point
      , std::vector<fhg::logging::endpoint> default_log_receivers
      , gspc::Certificates const&
      );
  }
}
