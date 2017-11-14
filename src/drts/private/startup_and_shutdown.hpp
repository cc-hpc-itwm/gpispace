// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <installation_path.hpp>

#include <fhg/util/signal_handler_manager.hpp>

#include <rif/entry_point.hpp>

#include <vmem/types.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
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

    enum class component_type {vmem, vmem_shared_cache, orchestrator, agent, worker};

    struct processes_storage : boost::noncopyable
    {
      std::mutex _guard;
      std::unordered_map < fhg::rif::entry_point
                         , std::unordered_map<std::string /*name*/, pid_t>
                         > _;

      processes_storage (std::ostream& info_output)
        : _info_output (info_output)
      {}

      ~processes_storage();
      std::unordered_map
        < rif::entry_point
        , std::pair< std::string /* kind */
                   , std::unordered_map<pid_t, std::exception_ptr>
                   >
        > shutdown_worker (std::vector<fhg::rif::entry_point> const&);

      void store (fhg::rif::entry_point const&, std::string const& name, pid_t);
      boost::optional<pid_t> pidof
        (fhg::rif::entry_point const&, std::string const& name);

    private:
      std::ostream& _info_output;
    };

    std::pair< std::unordered_set<fhg::rif::entry_point>
             , std::unordered_map<fhg::rif::entry_point, std::exception_ptr>
             > start_workers_for
      ( std::vector<fhg::rif::entry_point> const& entry_points
      , std::string master_name
      , fhg::drts::hostinfo_type master_hostinfo
      , fhg::drts::worker_description const& description
      , bool verbose
      , boost::optional<std::string> const& gui_host
      , boost::optional<unsigned short> const& gui_port
      , boost::optional<std::string> const& log_host
      , boost::optional<unsigned short> const& log_port
      , fhg::drts::processes_storage& processes
      , boost::optional<boost::filesystem::path> const& log_dir
      , boost::optional<boost::filesystem::path> const& gpi_socket
      , std::vector<boost::filesystem::path> const& app_path
      , gspc::installation_path const&
      , std::ostream& info_output
      );

    hostinfo_type startup
      ( boost::optional<std::string> const& gui_host
      , boost::optional<unsigned short> const& gui_port
      , boost::optional<std::string> const& log_host
      , boost::optional<unsigned short> const& log_port
      , bool gpi_enabled
      , bool verbose
      , boost::optional<boost::filesystem::path> gpi_socket
      , gspc::installation_path const&
      , bool delete_logfiles
      , fhg::util::signal_handler_manager& signal_handler_manager
      , boost::optional<std::chrono::seconds> vmem_startup_timeout
      , boost::optional<unsigned short> vmem_port
      , std::vector<fhg::rif::entry_point> const&
      , fhg::rif::entry_point const&
      , boost::optional<boost::filesystem::path> const& log_dir
      , processes_storage&
      , std::string& master_agent_name
      , fhg::drts::hostinfo_type& master_agent_hostinfo
      , std::ostream& info_output
      , boost::optional<intertwine::vmem::size_t> shared_cache_size
      );
  }
}
