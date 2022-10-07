// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <drts/drts.hpp>
#include <drts/private/startup_and_shutdown.hpp>

#include <gspc/iml/Client.hpp>
#include <gspc/iml/Rifs.hpp>
#include <gspc/iml/RuntimeSystem.hpp>

#include <logging/stream_emitter.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <exception>
#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gspc
{
  #if GSPC_WITH_IML
  struct iml_runtime_system
  {
    std::unique_ptr<iml::Rifs> rifds;
    std::unique_ptr<iml::RuntimeSystem> rts;

    iml_runtime_system ( ::boost::program_options::variables_map const& vm
                       , std::ostream& info_output
                       );
    iml_runtime_system (iml_runtime_system const&) = delete;
    iml_runtime_system (iml_runtime_system&&) = default;
    iml_runtime_system& operator= (iml_runtime_system const&) = delete;
    iml_runtime_system& operator= (iml_runtime_system&&) = default;
    ~iml_runtime_system() = default;
  };
  #else
  struct iml_runtime_system {};
  #endif

  struct scoped_runtime_system::implementation
  {
    implementation ( ::boost::program_options::variables_map const& vm
                   , installation const&
                   , std::string const& topology_description
                   , ::boost::optional<rifd_entry_points> const& entry_points
                   , rifd_entry_point const& parent
                   , std::ostream& info_output
                   , Certificates const& certificates
                   );

    std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
      add_worker (rifd_entry_points const&, Certificates const&);
    std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
      add_worker ( std::vector<worker_description> const&
                 , rifd_entry_points const&
                 , Certificates const&
                 );
    std::unordered_map< fhg::rif::entry_point
                      , std::pair< std::string /* kind */
                                 , std::unordered_map<pid_t, std::exception_ptr>
                                 >
                      >
      remove_worker (rifd_entry_points const&);

    ::boost::optional<::boost::filesystem::path> _virtual_memory_socket;

    ::boost::optional<iml_runtime_system> _iml_rts;

    struct started_runtime_system
    {
      started_runtime_system ( ::boost::optional<unsigned short> const& agent_port
                             , ::boost::optional<::boost::filesystem::path> gpi_socket
                             , std::vector<::boost::filesystem::path> app_path
                             , std::vector<std::string> worker_env_copy_variable
                             , bool worker_env_copy_current
                             , std::vector<::boost::filesystem::path> worker_env_copy_file
                             , std::vector<std::string> worker_env_set_variable
                             , installation_path
                             , std::vector<worker_description> worker_descriptions
                             , std::vector<fhg::rif::entry_point> const& rif_entry_points
                             , fhg::rif::entry_point const& parent
                             , std::ostream& info_output
                             , ::boost::optional<fhg::rif::entry_point> log_rif_entry_point
                             , std::vector<fhg::logging::endpoint> default_log_receivers
                             , Certificates const& certificates
                             );

      std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
        add_worker_impl
          ( std::vector<worker_description> const&
          , std::vector<fhg::rif::entry_point> const&
          , Certificates const&
          );
      std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
        add_worker (std::vector<fhg::rif::entry_point> const&, Certificates const&);
      std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
        add_worker
          ( std::vector<gspc::worker_description> const&
          , std::vector<fhg::rif::entry_point> const&
          , Certificates const&
          );
      std::unordered_map
        < fhg::rif::entry_point
        , std::pair< std::string /* kind */
                   , std::unordered_map<pid_t, std::exception_ptr>
                   >
        > remove_worker (std::vector<fhg::rif::entry_point> const&);

      std::ostream& _info_output;
      fhg::rif::entry_point _parent;
      ::boost::optional<::boost::filesystem::path> _gpi_socket;
      std::vector<::boost::filesystem::path> _app_path;
      std::vector<std::string> _worker_env_copy_variable;
      bool _worker_env_copy_current;
      std::vector<::boost::filesystem::path> _worker_env_copy_file;
      std::vector<std::string> _worker_env_set_variable;
      installation_path _installation_path;
      ::boost::optional<fhg::rif::entry_point> _logging_rif_entry_point;
      ::boost::optional<fhg::rif::protocol::start_logging_demultiplexer_result>
        _logging_rif_info;
      std::vector<worker_description> _worker_descriptions;

      fhg::drts::processes_storage _processes_storage;

      std::string _parent_agent_name;
      fhg::drts::hostinfo_type _parent_agent_hostinfo;

      std::string _top_level_agent_host;
      unsigned short _top_level_agent_port;
    } _started_runtime_system;
    fhg::logging::stream_emitter _logger;

    std::unique_ptr<iml::Client> _virtual_memory_api;
  };
}
