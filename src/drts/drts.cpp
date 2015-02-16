// mirko.rahn@itwm.fraunhofer.de

#include <drts/drts.hpp>
#include <drts/private/option.hpp>
#include <drts/private/rifd_entry_points_impl.hpp>
#include <drts/private/startup_and_shutdown.hpp>

#include <drts/stream.hpp>
#include <drts/virtual_memory.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/type/activity.hpp>
//! \todo eliminate this include (that completes type transition_t::data)
#include <we/type/net.hpp>

#include <gpi-space/pc/client/api.hpp>

#include <sdpa/client.hpp>

#include <fhg/util/hostname.hpp>
#include <fhg/util/make_unique.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/read_lines.hpp>
#include <fhg/util/split.hpp>
#include <fhg/revision.hpp>
#include <fhg/syscall.hpp>

#include <boost/format.hpp>

#include <chrono>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <unordered_set>

namespace gspc
{
  namespace
  {
    std::pair<std::list<std::string>, unsigned long>
      read_nodes (boost::filesystem::path const& nodefile)
    {
      std::unordered_set<std::string> unique_nodes;
      std::list<std::string> nodes;

      {
        std::ifstream stream (nodefile.string());

        std::string node;

        while (std::getline (stream, node))
        {
          unique_nodes.insert (node);
          nodes.emplace_back (node);
        }
      }

      if (unique_nodes.empty())
      {
        throw std::runtime_error
          (( boost::format ("nodefile %1% does not contain nodes")
           % nodefile
           ).str()
          );
      }

      return std::make_pair (nodes, unique_nodes.size());
    }
  }

  installation::installation
    (boost::program_options::variables_map const& vm)
      : _gspc_home (boost::filesystem::canonical (require_gspc_home (vm)))
  {
    boost::filesystem::path const path_revision (_gspc_home / "revision");

    if (!boost::filesystem::exists (path_revision))
    {
      throw std::invalid_argument
        ((boost::format ("File '%1%' does not exist.") % path_revision).str());
    }

    std::string const revision (fhg::util::read_file (path_revision));

    if (revision != fhg::project_revision())
    {
      throw std::invalid_argument
        (( boost::format ( "GSPC revision mismatch: Expected '%1%'"
                         ", installation in '%2%' has version '%3%'"
                         )
         % fhg::project_revision()
         % _gspc_home
         % revision
         ).str()
        );
    }
  }

  scoped_runtime_system::scoped_runtime_system
      ( boost::program_options::variables_map const& vm
      , installation const& installation
      , std::string const& topology_description
      )
    : scoped_runtime_system
        ( vm
        , installation
        , topology_description
        , require_rif_entry_points_file (vm)
        )
  {}

  scoped_runtime_system::scoped_runtime_system
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , std::string const& topology_description
    , rifd_entry_points const& entry_points
    )
      : _installation (installation)
      , _state_directory (require_state_directory (vm))
      , _virtual_memory_per_node (get_virtual_memory_per_node (vm))
      , _virtual_memory_socket (get_virtual_memory_socket (vm))
      , _virtual_memory_startup_timeout
        ( get_virtual_memory_startup_timeout (vm)
        ? boost::make_optional
          (std::chrono::seconds (get_virtual_memory_startup_timeout (vm).get()))
        : boost::none
        )
      , _nodes_and_number_of_unique_nodes
          (read_nodes (boost::filesystem::canonical (require_nodefile (vm))))
      , _virtual_memory_api
        ( _virtual_memory_socket
        ? fhg::util::make_unique<gpi::pc::client::api_t>
          (_virtual_memory_socket->string())
        : nullptr
        )
      , _rif_entry_points (entry_points)
  {
    unsigned short const default_log_port
      ((65535 - 30000 + fhg::syscall::getuid() * 2) % 65535 + 1024);
    unsigned short const default_gui_port (default_log_port + 1);

    std::vector<fhg::drts::worker_description> worker_descriptions;
    for ( std::string const& description
        : fhg::util::split<std::string, std::string> (topology_description, ' ')
        )
    {
      //! \todo configurable: default number of processes
      worker_descriptions.emplace_back
        (fhg::drts::parse_capability (1, description));
    }

    fhg::util::signal_handler_manager signal_handler_manager;

    std::tie (_orchestrator_host, _orchestrator_port) = fhg::drts::startup
      ( get_gui_host (vm).get_value_or (fhg::util::hostname())
      , get_gui_port (vm).get_value_or (default_gui_port)
      , get_log_host (vm).get_value_or (fhg::util::hostname())
      , get_log_port (vm).get_value_or (default_log_port)
      , _virtual_memory_per_node
      //! \todo configurable: verbose logging
      , false
      , _virtual_memory_socket
      , get_application_search_path (vm)
      ? std::vector<boost::filesystem::path> ({boost::filesystem::canonical (get_application_search_path (vm).get())})
      : std::vector<boost::filesystem::path>()
      , _installation.gspc_home()
      //! \todo configurable: number of segments
      , 1
      , _state_directory
      // !\todo configurable: delete logfiles
      , true
      , signal_handler_manager
      , _virtual_memory_per_node
      , _virtual_memory_startup_timeout
      , worker_descriptions
      , get_virtual_memory_port (vm)
      , _rif_entry_points._->_entry_points
      );

    if (_virtual_memory_per_node)
    {
      _virtual_memory_api->start();
    }
  }

  scoped_runtime_system::~scoped_runtime_system()
  {
    _virtual_memory_api.reset();

    fhg::drts::shutdown (_state_directory, _rif_entry_points._->_entry_points);
  }

  vmem_allocation scoped_runtime_system::alloc
    (unsigned long size, std::string const& description) const
  {
    return vmem_allocation (this, size, description);
  }
  vmem_allocation scoped_runtime_system::alloc_and_fill
    ( unsigned long size
    , std::string const& description
    , char const* const data
    ) const
  {
    return vmem_allocation (this, size, description, data);
  }

  stream scoped_runtime_system::create_stream ( std::string const& name
                                              , gspc::vmem_allocation const& buffer
                                              , stream::size_of_slot const& size_of_slot
                                              , std::function<void (pnet::type::value::value_type const&)> on_slot_filled
                                              ) const
  {
    return stream (*this, name, buffer, size_of_slot, on_slot_filled);
  }
}
