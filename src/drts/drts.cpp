// mirko.rahn@itwm.fraunhofer.de

#include <drts/drts.hpp>
#include <drts/private/drts_impl.hpp>
#include <drts/private/option.hpp>
#include <drts/private/pimpl.hpp>
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
#include <fhg/util/nest_exceptions.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/read_lines.hpp>
#include <fhg/util/split.hpp>
#include <fhg/revision.hpp>
#include <util-generic/syscall.hpp>

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
        (( boost::format ("GSPC revision mismatch: File '%1%' does not exist.")
         % path_revision
         ).str());
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
      : _ (new implementation (vm, installation, topology_description, entry_points))
  {}

  PIMPL_DTOR (scoped_runtime_system)


  namespace
  {
    std::vector<fhg::drts::worker_description> parse_worker_descriptions
      (std::string const& descriptions)
    {
      std::vector<fhg::drts::worker_description> worker_descriptions;
      for ( std::string const& description
          : fhg::util::split<std::string, std::string> (descriptions, ' ')
          )
      {
        //! \todo configurable: default number of processes
        worker_descriptions.emplace_back
          (fhg::drts::parse_capability (1, description));
      }
      return worker_descriptions;
    }
  }

  scoped_runtime_system::implementation::started_runtime_system::started_runtime_system
      ( boost::optional<std::string> const& gui_host
      , boost::optional<unsigned short> const& gui_port
      , boost::optional<std::string> const& log_host
      , boost::optional<unsigned short> const& log_port
      , bool gpi_enabled
      , bool verbose
      , boost::optional<boost::filesystem::path> gpi_socket
      , std::vector<boost::filesystem::path> app_path
      , boost::filesystem::path sdpa_home
      , boost::optional<boost::filesystem::path> const& log_dir
      , bool delete_logfiles
      , boost::optional<std::size_t> gpi_mem
      , boost::optional<std::chrono::seconds> vmem_startup_timeout
      , std::vector<fhg::drts::worker_description> worker_descriptions
      , boost::optional<unsigned short> vmem_port
      , std::vector<fhg::rif::entry_point> const& rif_entry_points
      )
    : _rif_entry_points (rif_entry_points) //! \note vmem started in startup
    , _gui_host (gui_host)
    , _gui_port (gui_port)
    , _log_host (log_host)
    , _log_port (log_port)
    , _verbose (verbose)
    , _gpi_socket (gpi_socket)
    , _app_path (app_path)
    , _sdpa_home (sdpa_home)
    , _log_dir (log_dir)
    , _worker_descriptions (worker_descriptions)
  {
    fhg::util::signal_handler_manager signal_handler_manager;

    std::tie (_orchestrator_host, _orchestrator_port) = fhg::drts::startup
      ( _gui_host
      , _gui_port
      , _log_host
      , _log_port
      , gpi_enabled
      , _verbose
      , _gpi_socket
      , _sdpa_home
      , delete_logfiles
      , signal_handler_manager
      , gpi_mem
      , vmem_startup_timeout
      , vmem_port
      , rif_entry_points
      , _log_dir
      , _processes_storage
      , _master_agent_name
      , _master_agent_hostinfo
      );

    add_worker_impl (_rif_entry_points);
  }


  void scoped_runtime_system::implementation::started_runtime_system::add_worker
    (std::vector<fhg::rif::entry_point> const& entry_points)
  {
    if (_gpi_socket)
    {
      throw std::logic_error ("add_worker while vmem is in use");
    }

    add_worker_impl (entry_points);
  }

  void scoped_runtime_system::implementation::started_runtime_system::add_worker_impl
    (std::vector<fhg::rif::entry_point> const& entry_points)
  {
    fhg::util::nest_exceptions<std::runtime_error>
      ( [&]
        {
          for ( fhg::drts::worker_description const& description
              : _worker_descriptions
              )
          {
            start_workers_for ( entry_points
                              , _master_agent_name
                              , _master_agent_hostinfo
                              , description
                              , _verbose
                              , _gui_host
                              , _gui_port
                              , _log_host
                              , _log_port
                              , _processes_storage
                              , _log_dir
                              , _gpi_socket
                              , _app_path
                              , _sdpa_home
                              );
          }
        }
      , "at least one worker could not be started!"
      );
  }

  void scoped_runtime_system::implementation::started_runtime_system::remove_worker
    (std::vector<fhg::rif::entry_point> const& entry_points)
  {
    _processes_storage.shutdown ( fhg::drts::component_type::worker
                                , entry_points
                                );
  }

  scoped_runtime_system::implementation::implementation
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , std::string const& topology_description
    , rifd_entry_points const& entry_points
    )
      : _virtual_memory_per_node (get_virtual_memory_per_node (vm))
      , _virtual_memory_socket (get_virtual_memory_socket (vm))
      , _virtual_memory_startup_timeout
        ( get_virtual_memory_startup_timeout (vm)
        ? boost::make_optional
          (std::chrono::seconds (get_virtual_memory_startup_timeout (vm).get()))
        : boost::none
        )
      , _nodes_and_number_of_unique_nodes
          (read_nodes (boost::filesystem::canonical (require_nodefile (vm))))
      , _started_runtime_system ( get_gui_host (vm)
                                , get_gui_port (vm)
                                , get_log_host (vm)
                                , get_log_port (vm)
                                , _virtual_memory_per_node
                                //! \todo configurable: verbose logging
                                , false
                                , _virtual_memory_socket
                                , get_application_search_path (vm)
                                ? std::vector<boost::filesystem::path> ({boost::filesystem::canonical (get_application_search_path (vm).get())})
                                : std::vector<boost::filesystem::path>()
                                , installation.gspc_home()
                                , get_log_directory (vm)
                                // !\todo configurable: delete logfiles
                                , true
                                , _virtual_memory_per_node
                                , _virtual_memory_startup_timeout
                                , parse_worker_descriptions (topology_description)
                                , get_virtual_memory_port (vm)
                                , entry_points._->_entry_points
                                )
      , _virtual_memory_api
        ( _virtual_memory_socket
        ? fhg::util::make_unique<gpi::pc::client::api_t>
          (_virtual_memory_socket->string())
        : nullptr
        )
  {}
  void scoped_runtime_system::implementation::add_worker
    (rifd_entry_points const& rifd_entry_points)
  {
    _started_runtime_system.add_worker (rifd_entry_points._->_entry_points);
  }
  void scoped_runtime_system::implementation::remove_worker
    (rifd_entry_points const& rifd_entry_points)
  {
    _started_runtime_system.remove_worker (rifd_entry_points._->_entry_points);
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

  unsigned long scoped_runtime_system::virtual_memory_total() const
  {
      return number_of_unique_nodes()
        * (*_->_virtual_memory_per_node - 32UL * (1UL << 20UL));
  }

  unsigned long scoped_runtime_system::number_of_unique_nodes() const
  {
    return _->_nodes_and_number_of_unique_nodes.second;
  }

  void scoped_runtime_system::add_worker
    (rifd_entry_points const& rifd_entry_points)
  {
    _->add_worker (rifd_entry_points);
  }
  void scoped_runtime_system::remove_worker
    (rifd_entry_points const& rifd_entry_points)
  {
    _->remove_worker (rifd_entry_points);
  }
}
