// mirko.rahn@itwm.fraunhofer.de

#ifndef DRTS_DRTS_HPP
#define DRTS_DRTS_HPP

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace gspc
{
  namespace options
  {
    namespace name
    {
      constexpr char const* const log_host {"log-host"};
      constexpr char const* const log_port {"log-port"};
      constexpr char const* const gui_host {"gui-host"};
      constexpr char const* const gui_port {"gui-port"};

      constexpr char const* const state_directory {"state-directory"};
      constexpr char const* const gspc_home {"gspc-home"};
      constexpr char const* const nodefile {"nodefile"};
      constexpr char const* const application_search_path
        {"application-search-path"};

      constexpr char const* const virtual_memory_manager
        {"virtual-memory-manager"};
      constexpr char const* const virtual_memory_per_node
        {"virtual-memory-per-node"};
      constexpr char const* const virtual_memory_socket
        {"virtual-memory-socket"};
    }

    boost::program_options::options_description logging();
    boost::program_options::options_description drts();
    boost::program_options::options_description virtual_memory();
  }

  struct scoped_runtime_system
  {
    scoped_runtime_system ( boost::program_options::variables_map const& vm
                          , std::string const& topology_description
                          );

    ~scoped_runtime_system();

    void put_and_run
      ( boost::filesystem::path const& workflow
      , std::unordered_map<std::string, std::string> const& values_on_ports
      ) const;

    boost::filesystem::path const& gspc_home() const
    {
      return _gspc_home;
    }
    boost::filesystem::path const& state_directory() const
    {
      return _state_directory;
    }
    boost::filesystem::path const& nodefile() const
    {
      return _nodefile;
    }
    boost::optional<unsigned long> const& virtual_memory_per_node() const
    {
      return _virtual_memory_per_node;
    }
    unsigned long virtual_memory_total() const
    {
      return _nodes.size() * (*_virtual_memory_per_node - 32UL * (1UL << 20UL));
    }
    boost::optional<boost::filesystem::path> const& virtual_memory_socket() const
    {
      return _virtual_memory_socket;
    }
    std::unordered_set<std::string> const& nodes() const
    {
      return _nodes;
    }

  private:
    boost::filesystem::path const _gspc_home;
    boost::filesystem::path const _state_directory;
    boost::filesystem::path const _nodefile;
    boost::optional<unsigned long> _virtual_memory_per_node;
    boost::optional<boost::filesystem::path> _virtual_memory_socket;
    std::unordered_set<std::string> _nodes;
  };
}

#endif
