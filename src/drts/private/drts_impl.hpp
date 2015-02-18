// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <drts/drts.hpp>

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

    ~implementation();

    installation const _installation;
    boost::filesystem::path const _state_directory;
    boost::optional<unsigned long> _virtual_memory_per_node;
    boost::optional<boost::filesystem::path> _virtual_memory_socket;
    boost::optional<std::chrono::seconds> _virtual_memory_startup_timeout;
    std::pair<std::list<std::string>, unsigned long> const
      _nodes_and_number_of_unique_nodes;
    std::unique_ptr<gpi::pc::client::api_t> _virtual_memory_api;

    rifd_entry_points _rif_entry_points;

    std::string _orchestrator_host;
    unsigned short _orchestrator_port;
  };
}
