// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_DRTS_PRIVATE_STARTUP_AND_SHUTDOWN_HPP
#define FHG_DRTS_PRIVATE_STARTUP_AND_SHUTDOWN_HPP

#include <fhg/util/signal_handler_manager.hpp>

#include <rif/entry_point.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <chrono>
#include <string>
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

    hostinfo_type startup
      ( std::string gui_host
      , unsigned short gui_port
      , std::string log_host
      , unsigned short log_port
      , bool gpi_enabled
      , bool verbose
      , boost::optional<boost::filesystem::path> gpi_socket
      , std::vector<boost::filesystem::path> app_path
      , boost::filesystem::path sdpa_home
      , std::size_t number_of_groups
      , boost::filesystem::path state_dir
      , bool delete_logfiles
      , fhg::util::signal_handler_manager& signal_handler_manager
      , boost::optional<std::size_t> gpi_mem
      , boost::optional<std::chrono::seconds> vmem_startup_timeout
      , std::vector<worker_description> worker_descriptions
      , boost::optional<unsigned short> vmem_port
      , std::vector<fhg::rif::entry_point> const&
      );

    void shutdown ( boost::filesystem::path const& state_dir
                  , std::vector<fhg::rif::entry_point> const&
                  );
  }
}

#endif
