// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <drts/drts.fwd.hpp>

#include <drts/information_to_reattach.fwd.hpp>
#include <drts/rifd_entry_points.hpp>
#include <drts/stream.hpp>
#include <drts/virtual_memory.fwd.hpp>

#include <we/type/value.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <chrono>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

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
    boost::program_options::options_description logging();
    boost::program_options::options_description installation();
    boost::program_options::options_description drts();
    boost::program_options::options_description external_rifd();
    boost::program_options::options_description scoped_rifd();
    boost::program_options::options_description virtual_memory();
  }

  class installation
  {
  public:
    installation (boost::program_options::variables_map const& vm);

    boost::filesystem::path const& gspc_home() const
    {
      return _gspc_home;
    }

  private:
    boost::filesystem::path const _gspc_home;
  };

  class scoped_runtime_system
  {
  public:
    scoped_runtime_system ( boost::program_options::variables_map const& vm
                          , installation const&
                          , std::string const& topology_description
                          );
    scoped_runtime_system ( boost::program_options::variables_map const& vm
                          , installation const&
                          , std::string const& topology_description
                          , rifd_entry_points const& entry_points
                          );

    ~scoped_runtime_system();

    vmem_allocation alloc
      (unsigned long size, std::string const& description) const;
    vmem_allocation alloc_and_fill
      ( unsigned long size
      , std::string const& description
      , char const* const data
      ) const;

    unsigned long virtual_memory_total() const;
    unsigned long number_of_unique_nodes() const;

    stream create_stream ( std::string const& name
                         , gspc::vmem_allocation const& buffer
                         , gspc::stream::size_of_slot const&
                         , std::function<void (pnet::type::value::value_type const&)> on_slot_filled
                         ) const;

    scoped_runtime_system (scoped_runtime_system const&) = delete;
    scoped_runtime_system& operator= (scoped_runtime_system const&) = delete;
    scoped_runtime_system (scoped_runtime_system&&) = delete;
    scoped_runtime_system& operator= (scoped_runtime_system&&) = delete;

  private:
    std::unique_ptr<gpi::pc::client::api_t> const& virtual_memory_api() const;

    friend class vmem_allocation;
    friend class information_to_reattach;
    friend class stream;

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

  void set_application_search_path ( boost::program_options::variables_map&
                                   , boost::filesystem::path const&
                                   );
  void set_gspc_home ( boost::program_options::variables_map&
                     , boost::filesystem::path const&
                     );
}
