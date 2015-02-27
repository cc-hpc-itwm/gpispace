// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <fhg/util/signal_handler_manager.hpp>

#include <rif/entry_point.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

namespace fhg
{
  namespace drts
  {
    struct worker_description
    {
      std::vector<std::string> capabilities;
      std::size_t num_per_node;
      std::size_t max_nodes;
      std::size_t shm_size;
      boost::optional<std::size_t> socket;
    };
    worker_description parse_capability
      (std::size_t def_num_proc, std::string const& cap_spec);

    using hostinfo_type = std::pair<std::string, unsigned short>;

    enum class component_type {vmem, orchestrator, agent, worker};

    struct processes_storage : boost::noncopyable
    {
      std::unordered_map < fhg::rif::entry_point
                         , std::unordered_map<std::string /*name*/, pid_t>
                         > _;

      ~processes_storage();
      void shutdown (component_type, std::vector<fhg::rif::entry_point> const&);

      void store (fhg::rif::entry_point const&, std::string const& name, pid_t);
    };

    hostinfo_type startup
      ( boost::optional<std::string> const& gui_host
      , boost::optional<unsigned short> const& gui_port
      , boost::optional<std::string> const& log_host
      , boost::optional<unsigned short> const& log_port
      , bool gpi_enabled
      , bool verbose
      , boost::optional<boost::filesystem::path> gpi_socket
      , std::vector<boost::filesystem::path> app_path
      , boost::filesystem::path sdpa_home
      , bool delete_logfiles
      , fhg::util::signal_handler_manager& signal_handler_manager
      , boost::optional<std::size_t> gpi_mem
      , boost::optional<std::chrono::seconds> vmem_startup_timeout
      , std::vector<worker_description> worker_descriptions
      , boost::optional<unsigned short> vmem_port
      , std::vector<fhg::rif::entry_point> const&
      , boost::optional<boost::filesystem::path> const& log_dir
      , processes_storage&
      );
  }
}
