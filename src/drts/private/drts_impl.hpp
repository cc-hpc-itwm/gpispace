// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <drts/drts.hpp>
#include <drts/private/startup_and_shutdown.hpp>

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
                   , rifd_entry_points const& entry_points
                   );

    void add_worker (rifd_entry_points const&);
    void remove_worker (rifd_entry_points const&);

    boost::optional<unsigned long> _virtual_memory_per_node;
    boost::optional<boost::filesystem::path> _virtual_memory_socket;
    boost::optional<std::chrono::seconds> _virtual_memory_startup_timeout;
    std::pair<std::list<std::string>, unsigned long> const
      _nodes_and_number_of_unique_nodes;

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
                             , boost::filesystem::path sdpa_home
                             , boost::optional<boost::filesystem::path> const& log_dir
                             , bool delete_logfiles
                             , boost::optional<std::size_t> gpi_mem
                             , boost::optional<std::chrono::seconds> vmem_startup_timeout
                             , std::vector<fhg::drts::worker_description> worker_descriptions
                             , boost::optional<unsigned short> vmem_port
                             , std::vector<fhg::rif::entry_point> const& rif_entry_points
                             );

      void add_worker_impl (std::vector<fhg::rif::entry_point> const&);
      void add_worker (std::vector<fhg::rif::entry_point> const&);
      void remove_worker (std::vector<fhg::rif::entry_point> const&);

      std::vector<fhg::rif::entry_point> _rif_entry_points;
      boost::optional<std::string> _gui_host;
      boost::optional<unsigned short> _gui_port;
      boost::optional<std::string> _log_host;
      boost::optional<unsigned short> _log_port;
      bool _verbose;
      boost::optional<boost::filesystem::path> _gpi_socket;
      std::vector<boost::filesystem::path> _app_path;
      boost::filesystem::path _sdpa_home;
      boost::optional<boost::filesystem::path> _log_dir;
      std::vector<fhg::drts::worker_description> _worker_descriptions;

      fhg::drts::processes_storage _processes_storage;

      std::string _master_agent_name;
      fhg::drts::hostinfo_type _master_agent_hostinfo;

      std::string _orchestrator_host;
      unsigned short _orchestrator_port;
    } _started_runtime_system;
    std::unique_ptr<gpi::pc::client::api_t> _virtual_memory_api;
  };
}