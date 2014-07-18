// mirko.rahn@itwm.fraunhofer.de

#ifndef DRTS_DRTS_HPP
#define DRTS_DRTS_HPP

#include <drts/virtual_memory.hpp>

#include <we/type/value.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace gpi
{
  namespace pc
  {
    namespace client
    {
      class api_t;
    }
  }
}

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

  class scoped_runtime_system
  {
  public:
    scoped_runtime_system ( boost::program_options::variables_map const& vm
                          , std::string const& topology_description
                          );

    ~scoped_runtime_system();

    std::multimap<std::string, pnet::type::value::value_type>
      put_and_run
      ( boost::filesystem::path const& workflow
      , std::unordered_map<std::string, std::unordered_set<std::string>> const&
          values_on_ports
      ) const;

    vmem_allocation alloc
      (unsigned long size, std::string const& description) const;

    boost::filesystem::path const& gspc_home() const
    {
      return _gspc_home;
    }
    unsigned long virtual_memory_total() const
    {
      return _nodes.size() * (*_virtual_memory_per_node - 32UL * (1UL << 20UL));
    }
    std::unordered_set<std::string> const& nodes() const
    {
      return _nodes;
    }

    scoped_runtime_system (scoped_runtime_system const&) = delete;
    scoped_runtime_system& operator= (scoped_runtime_system const&) = delete;
    scoped_runtime_system (scoped_runtime_system&&) = delete;
    scoped_runtime_system& operator= (scoped_runtime_system&&) = delete;

  private:
    friend class vmem_allocation;

    boost::filesystem::path const _gspc_home;
    boost::filesystem::path const _state_directory;
    boost::filesystem::path const _nodefile;
    boost::optional<unsigned long> _virtual_memory_per_node;
    boost::optional<boost::filesystem::path> _virtual_memory_socket;
    std::unordered_set<std::string> _nodes;
    gpi::pc::client::api_t* _virtual_memory_api;
  };

  void set_gspc_home ( boost::program_options::variables_map&
                     , boost::filesystem::path const&
                     );
  void set_state_directory ( boost::program_options::variables_map&
                           , boost::filesystem::path const&
                           );
  void set_nodefile ( boost::program_options::variables_map&
                    , boost::filesystem::path const&
                    );
  void set_virtual_memory_manager ( boost::program_options::variables_map&
                                  , boost::filesystem::path const&
                                  );
  void set_virtual_memory_per_node ( boost::program_options::variables_map&
                                   , unsigned long
                                   );
  void set_virtual_memory_socket ( boost::program_options::variables_map&
                                 , boost::filesystem::path const&
                                 );
  void set_application_search_path ( boost::program_options::variables_map&
                                   , boost::filesystem::path const&
                                   );
  void set_log_host ( boost::program_options::variables_map&
                    , std::string const&
                    );
  void set_gui_host ( boost::program_options::variables_map&
                    , std::string const&
                    );
  void set_log_port ( boost::program_options::variables_map&
                    , unsigned short
                    );
  void set_gui_port ( boost::program_options::variables_map&
                    , unsigned short
                    );
}

#endif
