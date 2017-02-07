// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <drts/drts.hpp>
#include <drts/private/startup_and_shutdown.hpp>

#include <vmem/ipc_client.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <chrono>
#include <list>
#include <memory>
#include <string>

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
                   );

    std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
      add_worker (rifd_entry_points const&);
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
      started_runtime_system ( boost::optional<std::string> const& gui_host
                             , boost::optional<unsigned short> const& gui_port
                             , boost::optional<std::string> const& log_host
                             , boost::optional<unsigned short> const& log_port
                             , bool gpi_enabled
                             , bool verbose
                             , boost::optional<boost::filesystem::path> gpi_socket
                             , std::vector<boost::filesystem::path> app_path
                             , installation_path
                             , boost::optional<boost::filesystem::path> const& log_dir
                             , bool delete_logfiles
                             , boost::optional<std::chrono::seconds> vmem_startup_timeout
                             , std::vector<fhg::drts::worker_description> worker_descriptions
                             , boost::optional<unsigned short> vmem_port
                             , std::vector<fhg::rif::entry_point> const& rif_entry_points
                             , fhg::rif::entry_point const& master
                             , std::ostream& info_output
                             );

      std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
        add_worker_impl (std::vector<fhg::rif::entry_point> const&);
      std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
        add_worker (std::vector<fhg::rif::entry_point> const&);
      std::unordered_map
        < fhg::rif::entry_point
        , std::pair< std::string /* kind */
                   , std::unordered_map<pid_t, std::exception_ptr>
                   >
        > remove_worker (std::vector<fhg::rif::entry_point> const&);

      std::ostream& _info_output;
      fhg::rif::entry_point _master;
      boost::optional<std::string> _gui_host;
      boost::optional<unsigned short> _gui_port;
      boost::optional<std::string> _log_host;
      boost::optional<unsigned short> _log_port;
      bool _verbose;
      boost::optional<boost::filesystem::path> _gpi_socket;
      std::vector<boost::filesystem::path> _app_path;
      installation_path _installation_path;
      boost::optional<boost::filesystem::path> _log_dir;
      std::vector<fhg::drts::worker_description> _worker_descriptions;

      fhg::drts::processes_storage _processes_storage;

      std::string _master_agent_name;
      fhg::drts::hostinfo_type _master_agent_hostinfo;

      std::string _orchestrator_host;
      unsigned short _orchestrator_port;
    } _started_runtime_system;
    fhg::log::Logger _logger;
    std::unique_ptr<intertwine::vmem::ipc_client> _virtual_memory_api;
  };
}
