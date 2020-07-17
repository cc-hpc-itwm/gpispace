#pragma once

#include <drts/drts.hpp>
#include <drts/private/startup_and_shutdown.hpp>

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
  struct scoped_runtime_system::implementation
  {
    implementation ( boost::program_options::variables_map const& vm
                   , installation const&
                   , std::string const& topology_description
                   , boost::optional<rifd_entry_points> const& entry_points
                   , rifd_entry_point const& master
                   , std::ostream& info_output
                   , Certificates const& certificates
                   , boost::optional<UniqueForest<resource::Class>> const& resources
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

    boost::optional<boost::filesystem::path> _virtual_memory_socket;
    boost::optional<std::chrono::seconds> _virtual_memory_startup_timeout;

    struct started_runtime_system
    {
      started_runtime_system ( boost::optional<unsigned short> const& agent_port
                             , bool gpi_enabled
                             , boost::optional<boost::filesystem::path> gpi_socket
                             , std::vector<boost::filesystem::path> app_path
                             , std::vector<std::string> worker_env_copy_variable
                             , bool worker_env_copy_current
                             , std::vector<boost::filesystem::path> worker_env_copy_file
                             , std::vector<std::string> worker_env_set_variable
                             , installation_path
                             , boost::optional<std::chrono::seconds> vmem_startup_timeout
                             , std::string const& topology_description
                             , boost::optional<unsigned short> vmem_port
                             , boost::optional<fhg::vmem::netdev_id> vmem_netdev_id
                             , std::vector<fhg::rif::entry_point> const& rif_entry_points
                             , fhg::rif::entry_point const& master
                             , std::ostream& info_output
                             , boost::optional<fhg::rif::entry_point> log_rif_entry_point
                             , std::vector<fhg::logging::endpoint> default_log_receivers
                             , Certificates const& certificates
                             , boost::optional<UniqueForest<resource::Class>> const& resources
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
      fhg::rif::entry_point _master;
      boost::optional<boost::filesystem::path> _gpi_socket;
      std::vector<boost::filesystem::path> _app_path;
      std::vector<std::string> _worker_env_copy_variable;
      bool _worker_env_copy_current;
      std::vector<boost::filesystem::path> _worker_env_copy_file;
      std::vector<std::string> _worker_env_set_variable;
      installation_path _installation_path;
      boost::optional<fhg::rif::entry_point> _logging_rif_entry_point;
      boost::optional<fhg::rif::protocol::start_logging_demultiplexer_result>
        _logging_rif_info;
      std::vector<worker_description> _worker_descriptions;

      fhg::drts::processes_storage _processes_storage;

      std::string _master_agent_name;
      fhg::drts::hostinfo_type _master_agent_hostinfo;

      std::string _top_level_agent_host;
      unsigned short _top_level_agent_port;
    } _started_runtime_system;
    fhg::logging::stream_emitter _logger;
    std::unique_ptr<gpi::pc::client::api_t> _virtual_memory_api;
  };
}
