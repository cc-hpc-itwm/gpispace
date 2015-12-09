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

#include <util-generic/hostname.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/split.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>
#include <fhg/revision.hpp>
#include <util-generic/syscall.hpp>

#include <boost/format.hpp>

#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace gspc
{
  installation::installation
    (boost::filesystem::path const& gspc_home)
      : _gspc_home (boost::filesystem::canonical (gspc_home))
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
  installation::installation
    (boost::program_options::variables_map const& vm)
      : installation (require_gspc_home (vm))
  {}

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
      : scoped_runtime_system
          ( vm
          , installation
          , topology_description
          , entry_points
          , [&entry_points]() -> rifd_entry_point
            {
              if (entry_points._->_entry_points.empty())
              {
                throw std::logic_error
                  ("scoped_runtime_system: no entry_points given");
              }

              return { new rifd_entry_point::implementation
                         (entry_points._->_entry_points.front())
                     };
            }()
          )
  {}
  scoped_runtime_system::scoped_runtime_system
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , std::string const& topology_description
    , boost::optional<rifd_entry_points> const& entry_points
    , rifd_entry_point const& master
    )
      : scoped_runtime_system
          ( vm
          , installation
          , topology_description
          , entry_points
          , master
          , std::cerr
          )
  {}
  scoped_runtime_system::scoped_runtime_system
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , std::string const& topology_description
    , boost::optional<rifd_entry_points> const& entry_points
    , rifd_entry_point const& master
    , std::ostream& info_output
    )
      : _ (new implementation ( vm
                              , installation
                              , topology_description
                              , entry_points
                              , master
                              , info_output
                              )
          )
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
      , boost::optional<std::chrono::seconds> vmem_startup_timeout
      , std::vector<fhg::drts::worker_description> worker_descriptions
      , boost::optional<unsigned short> vmem_port
      , std::vector<fhg::rif::entry_point> const& rif_entry_points
      , fhg::rif::entry_point const& master
      , std::ostream& info_output
      )
    : _info_output (info_output)
    , _master (master)
    , _rif_entry_points (rif_entry_points) //! \note vmem started in startup
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
    , _processes_storage (_info_output)
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
      , vmem_startup_timeout
      , vmem_port
      , _rif_entry_points
      , _master
      , _log_dir
      , _processes_storage
      , _master_agent_name
      , _master_agent_hostinfo
      , _info_output
      );

    if (!_rif_entry_points.empty())
    {
       std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
         const failures (add_worker_impl (_rif_entry_points));

      if (!failures.empty())
      {
        fhg::util::throw_collected_exceptions
          ( failures
          , [] (std::pair<fhg::rif::entry_point, std::list<std::exception_ptr>>
                 const& fail
               )
            {
              std::ostringstream oss;

              oss << "drts-kernel startup failed on '" << fail.first << "':";

              for (std::exception_ptr const& exception_ptr : fail.second)
              {
                oss << ' ' << fhg::util::exception_printer (exception_ptr);
              }

              return oss.str();
            }
          );
      }
    }
  }

  std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
    scoped_runtime_system::implementation::started_runtime_system::add_worker
      (std::vector<fhg::rif::entry_point> const& entry_points)
  {
    if (_gpi_socket)
    {
      throw std::logic_error ("add_worker while vmem is in use");
    }

    return add_worker_impl (entry_points);
  }

  std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
    scoped_runtime_system::implementation::started_runtime_system::add_worker_impl
    (std::vector<fhg::rif::entry_point> const& entry_points)
  {
    std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
      failures;

    for ( fhg::drts::worker_description const& description
        : _worker_descriptions
        )
    {
      for (auto const& fail : start_workers_for ( entry_points
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
                                                , _info_output
                                                ).second
          )
      {
        failures[fail.first].emplace_back (fail.second);
      }
    }

    return failures;
  }

  std::unordered_map< fhg::rif::entry_point
                    , std::pair< std::string /* kind */
                               , std::unordered_map<pid_t, std::exception_ptr>
                               >
                    >
    scoped_runtime_system::implementation::started_runtime_system::remove_worker
      (std::vector<fhg::rif::entry_point> const& entry_points)
  {
    return _processes_storage.shutdown_worker (entry_points);
  }

  scoped_runtime_system::implementation::implementation
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , std::string const& topology_description
    , boost::optional<rifd_entry_points> const& entry_points
    , rifd_entry_point const& master
    , std::ostream& info_output
    )
      : _virtual_memory_socket (get_virtual_memory_socket (vm))
      , _virtual_memory_startup_timeout
        ( get_virtual_memory_startup_timeout (vm)
        ? boost::make_optional
          (std::chrono::seconds (get_virtual_memory_startup_timeout (vm).get()))
        : boost::none
        )
      , _nodes_and_number_of_unique_nodes
          ( !entry_points
            ? decltype (_nodes_and_number_of_unique_nodes) {{}, 0}
            : entry_points->_->nodes_and_number_of_unique_nodes()
          )
      , _started_runtime_system
          ( get_gui_host (vm)
          , get_gui_port (vm)
          , get_log_host (vm)
          , get_log_port (vm)
          , !!_virtual_memory_socket
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
          , _virtual_memory_startup_timeout
          , parse_worker_descriptions (topology_description)
          , get_virtual_memory_port (vm)
          , !entry_points
            ? decltype (entry_points->_->_entry_points) {}
            : entry_points->_->_entry_points
          , master._->_entry_point
          , info_output
          )
      , _logger()
      , _virtual_memory_api
        ( _virtual_memory_socket
        ? fhg::util::cxx14::make_unique<gpi::pc::client::api_t>
            (_logger, _virtual_memory_socket->string())
        : nullptr
        )
  {}
  std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
    scoped_runtime_system::implementation::add_worker
      (rifd_entry_points const& rifd_entry_points)
  {
    return _started_runtime_system.add_worker
      (rifd_entry_points._->_entry_points);
  }
  std::unordered_map< fhg::rif::entry_point
                    , std::pair< std::string /* kind */
                               , std::unordered_map<pid_t, std::exception_ptr>
                               >
                    >
      scoped_runtime_system::implementation::remove_worker
    (rifd_entry_points const& rifd_entry_points)
  {
    return _started_runtime_system
      .remove_worker (rifd_entry_points._->_entry_points);
  }

  vmem_allocation scoped_runtime_system::alloc
    ( vmem::segment_description segment_description
    , unsigned long size
    , std::string const& name
    ) const
  {
    return vmem_allocation (this, segment_description, size, name);
  }
  vmem_allocation scoped_runtime_system::alloc_and_fill
    ( vmem::segment_description segment_description
    , unsigned long size
    , std::string const& name
    , char const* const data
    ) const
  {
    return vmem_allocation (this, segment_description, size, name, data);
  }

  stream scoped_runtime_system::create_stream ( std::string const& name
                                              , gspc::vmem_allocation const& buffer
                                              , stream::size_of_slot const& size_of_slot
                                              , std::function<void (pnet::type::value::value_type const&)> on_slot_filled
                                              ) const
  {
    return stream (*this, name, buffer, size_of_slot, on_slot_filled);
  }

  unsigned long scoped_runtime_system::number_of_unique_nodes() const
  {
    return _->_nodes_and_number_of_unique_nodes.second;
  }

  std::unordered_map< rifd_entry_point
                    , std::list<std::exception_ptr>
                    , rifd_entry_point_hash
                    >
    scoped_runtime_system::add_worker
      (rifd_entry_points const& rifd_entry_points)
  {
    std::unordered_map< fhg::rif::entry_point
                      , std::list<std::exception_ptr>
                      > const result (_->add_worker (rifd_entry_points));
    std::unordered_map< rifd_entry_point
                      , std::list<std::exception_ptr>
                      , rifd_entry_point_hash
                      > wrapped;

    for (auto const& x : result)
    {
      wrapped.emplace
        (new rifd_entry_point::implementation (x.first), x.second);
    }

    return wrapped;
  }
  std::unordered_map< rifd_entry_point
                    , std::pair< std::string /* kind */
                               , std::unordered_map<pid_t, std::exception_ptr>
                               >
                    , rifd_entry_point_hash
                    >
    scoped_runtime_system::remove_worker
      (rifd_entry_points const& rifd_entry_points)
  {
    std::unordered_map< fhg::rif::entry_point
                      , std::pair< std::string /* kind */
                                 , std::unordered_map<pid_t, std::exception_ptr>
                                 >
                      > const result (_->remove_worker (rifd_entry_points));
    std::unordered_map< rifd_entry_point
                      , std::pair< std::string /* kind */
                                 , std::unordered_map<pid_t, std::exception_ptr>
                                 >
                      , rifd_entry_point_hash
                      > wrapped;

    for (auto const& x : result)
    {
      wrapped.emplace
        (new rifd_entry_point::implementation (x.first), x.second);
    }

    return wrapped;
  }
}
