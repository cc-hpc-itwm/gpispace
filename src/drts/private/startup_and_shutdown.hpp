#pragma once

#include <drts/certificates.hpp>
#include <drts/worker_description.hpp>
#include <drts/drts.fwd.hpp>

#include <installation_path.hpp>

#include <fhg/util/signal_handler_manager.hpp>

#include <logging/endpoint.hpp>

#include <rif/entry_point.hpp>
#include <rif/protocol.hpp>

#include <vmem/netdev_id.hpp>

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
    gspc::worker_description parse_capability
      (std::size_t def_num_proc, std::string const& cap_spec);

    using hostinfo_type = std::pair<std::string, unsigned short>;

    enum class component_type
    {
      vmem,
      orchestrator,
      agent,
      worker,
      logging_demultiplexer,
    };

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
      , gspc::worker_description const& description
      , fhg::drts::processes_storage& processes
      , boost::optional<boost::filesystem::path> const& gpi_socket
      , std::vector<boost::filesystem::path> const& app_path
      , gspc::installation_path const&
      , std::ostream& info_output
      , boost::optional<std::pair<fhg::rif::entry_point, pid_t>> top_level_log
      , gspc::Certificates const& certificates
      );

    struct startup_result
    {
      hostinfo_type orchestrator;
      boost::optional<rif::protocol::start_logging_demultiplexer_result>
        top_level_logging_demultiplexer;
    };

    startup_result startup
      ( boost::optional<unsigned short> const& orchestrator_port
      , boost::optional<unsigned short> const& agent_port
      , bool gpi_enabled
      , boost::optional<boost::filesystem::path> gpi_socket
      , gspc::installation_path const&
      , fhg::util::signal_handler_manager& signal_handler_manager
      , boost::optional<std::chrono::seconds> vmem_startup_timeout
      , boost::optional<unsigned short> vmem_port
      , boost::optional<vmem::netdev_id> vmem_netdev_id
      , std::vector<fhg::rif::entry_point> const&
      , fhg::rif::entry_point const&
      , processes_storage&
      , std::string& master_agent_name
      , fhg::drts::hostinfo_type& master_agent_hostinfo
      , std::ostream& info_output
      , boost::optional<fhg::rif::entry_point> log_rif_entry_point
      , std::vector<fhg::logging::endpoint> default_log_receivers
      , gspc::Certificates const& certificates
      );
  }
}
