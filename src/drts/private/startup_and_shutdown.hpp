// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <fhg/util/signal_handler_manager.hpp>

#include <rif/entry_point.hpp>

#include <boost/filesystem/path.hpp>
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

    struct processes_storage
    {
      std::unordered_map < fhg::rif::entry_point
                         , std::unordered_map<std::string /*name*/, pid_t>
                         > _;

      void store (fhg::rif::entry_point const&, std::string const& name, pid_t);
      void garbage_collect();
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
      , std::size_t number_of_groups
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

    //! \todo learn enum class
    namespace components_type
    {
      enum components_type
      {
        vmem = 1 << 1,
        orchestrator = 1 << 2,
        agent = 1 << 3,
        worker = 1 << 4,
      };
      constexpr const char* to_string (components_type const& component)
      {
        return component == worker ? "drts-kernel"
          : component == agent ? "agent"
          : component == orchestrator ? "orchestrator"
          : component == vmem ? "vmem"
          : throw std::logic_error ("invalid enum value");
      }
    }

    void shutdown ( processes_storage&
                  , boost::optional<components_type::components_type>
                  , std::vector<fhg::rif::entry_point> const&
                  );

    void shutdown ( std::vector<fhg::rif::entry_point> const&
                  , processes_storage&
                  );
  }
}
