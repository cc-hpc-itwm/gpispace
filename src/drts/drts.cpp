#include <drts/drts.hpp>
#include <drts/private/drts_impl.hpp>
#include <drts/private/option.hpp>
#include <drts/private/pimpl.hpp>
#include <drts/private/rifd_entry_points_impl.hpp>
#include <drts/private/startup_and_shutdown.hpp>

#include <drts/stream.hpp>
#include <drts/virtual_memory.hpp>

#include <we/type/activity.hpp>

#include <gpi-space/pc/client/api.hpp>

#include <sdpa/client.hpp>

#include <fhg/revision.hpp>
#include <fhg/util/boost/program_options/require_all_if_one.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/make_optional.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/split.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <boost/format.hpp>

#include <ostream>
#include <sstream>
#include <stdexcept>

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
      , std::ostream& info_output
      , Certificates const& certificates
      )
    : scoped_runtime_system
        ( vm
        , installation
        , topology_description
        , require_rif_entry_points_file (vm)
        , info_output
        , certificates
        )
  {}
  scoped_runtime_system::scoped_runtime_system
      ( boost::program_options::variables_map const& vm
      , installation const& installation
      , UniqueForest<Resource> const& resource_descriptions
      , std::ostream& info_output
      , Certificates const& certificates
      )
    : scoped_runtime_system
        ( vm
        , installation
        , resource_descriptions
        , require_rif_entry_points_file (vm)
        , info_output
        , certificates
        )
  {}
  scoped_runtime_system::scoped_runtime_system
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , std::string const& topology_description
    , rifd_entry_points const& entry_points
    , std::ostream& info_output
    , Certificates const& certificates
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
          , info_output
          , certificates
          )
  {}
  scoped_runtime_system::scoped_runtime_system
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , UniqueForest<Resource> const& resource_descriptions
    , rifd_entry_points const& entry_points
    , std::ostream& info_output
    , Certificates const& certificates
    )
      : scoped_runtime_system
          ( vm
          , installation
          , resource_descriptions
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
          , info_output
          , certificates
          )
  {}
  scoped_runtime_system::scoped_runtime_system
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , std::string const& topology_description
    , boost::optional<rifd_entry_points> const& entry_points
    , rifd_entry_point const& master
    , std::ostream& info_output
    , Certificates const& certificates
    )
      : _ (new implementation ( vm
                              , installation
                              , topology_description
                              , entry_points
                              , master
                              , info_output
                              , certificates
                              , boost::none
                              )
          )
  {}

  scoped_runtime_system::scoped_runtime_system
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , UniqueForest<Resource> const& resource_descriptions
    , boost::optional<rifd_entry_points> const& entry_points
    , rifd_entry_point const& master
    , std::ostream& info_output
    , Certificates const& certificates
    )
      : _ (new implementation ( vm
                              , installation
                              , ""
                              , entry_points
                              , master
                              , info_output
                              , certificates
                              , resource_descriptions
                              )
          )
  {}

  PIMPL_DTOR (scoped_runtime_system)


  namespace
  {
    std::vector<worker_description> parse_worker_descriptions
      (std::string const& descriptions)
    {
      std::vector<worker_description> worker_descriptions;
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
      ( boost::optional<unsigned short> const& agent_port
      , bool gpi_enabled
      , boost::optional<boost::filesystem::path> gpi_socket
      , std::vector<boost::filesystem::path> app_path
      , std::vector<std::string> worker_env_copy_variable
      , bool worker_env_copy_current
      , std::vector<boost::filesystem::path> worker_env_copy_file
      , std::vector<std::string> worker_env_set_variable
      , gspc::installation_path installation_path
      , boost::optional<std::chrono::seconds> vmem_startup_timeout
      , std::string const& topology_description
      , boost::optional<unsigned short> vmem_port
      , boost::optional<fhg::vmem::netdev_id> vmem_netdev_id
      , std::vector<fhg::rif::entry_point> const& rif_entry_points
      , fhg::rif::entry_point const& master
      , std::ostream& info_output
      , boost::optional<fhg::rif::entry_point> logging_rif_entry_point
      , std::vector<fhg::logging::endpoint> default_log_receivers
      , Certificates const& certificates
      , boost::optional<UniqueForest<Resource>> const&
          resource_descriptions
      )
    : _info_output (info_output)
    , _master (master)
    , _gpi_socket (gpi_socket)
    , _app_path (app_path)
    , _worker_env_copy_variable (std::move (worker_env_copy_variable))
    , _worker_env_copy_current (worker_env_copy_current)
    , _worker_env_copy_file (std::move (worker_env_copy_file))
    , _worker_env_set_variable (std::move (worker_env_set_variable))
    , _installation_path (installation_path)
    , _logging_rif_entry_point (logging_rif_entry_point)
    , _processes_storage (_info_output)
  {
    fhg::util::signal_handler_manager signal_handler_manager;

    using Resources = Forest<resource::ID, resource::Class>;
    Resources resources;
    using Worker_descriptions = Forest<resource::ID, worker_description>;
    Worker_descriptions worker_descriptions;

    if (resource_descriptions)
    {
      // assemble forest and create worker descriptions
      remote_interface::ID next_remote_interface_id {0};

      for (auto const& entry_point : rif_entry_points)
      {
        resource::ID next_resource_id {{next_remote_interface_id}};

        worker_descriptions.UNSAFE_merge
          ( resource_descriptions->unordered_transform
              ( [&] (unique_forest::Node<Resource> const& r) -> Worker_descriptions::Node
                {
                  std::vector<std::string> capabilities;
                  for ( std::string const& cpb
                      : fhg::util::split<std::string, std::string> (r.second.resource_class, '+')
                      )
                  {
                    capabilities.emplace_back (cpb);
                  }

                  worker_description const description
                    { capabilities
                    , 1
                    , 0
                    , r.second.shm_size
                    , boost::none
                    , boost::none
                    , ++next_resource_id
                    , entry_point
                    };

                  return {next_resource_id, description};
                }
              )
          );

        ++next_remote_interface_id;
      }
    }

    auto const startup_result
      ( fhg::drts::startup
          ( agent_port
          , gpi_enabled
          , _gpi_socket
          , _installation_path
          , signal_handler_manager
          , vmem_startup_timeout
          , vmem_port
          , vmem_netdev_id
          , rif_entry_points
          , _master
          , _processes_storage
          , _master_agent_name
          , _master_agent_hostinfo
          , _info_output
          , _logging_rif_entry_point
          , default_log_receivers
          , certificates
          , boost::make_optional
              ( !!resource_descriptions
              , worker_descriptions.unordered_transform
                ( [&] (Worker_descriptions::Node const& r) -> Resources::Node
                  {
                    return
                      { r.first
                      , fhg::util::join (r.second.capabilities, '+').string()
                      };
                  }
                )
              )
          )
      );
    _top_level_agent_host = startup_result.top_level_agent.first;
    _top_level_agent_port = startup_result.top_level_agent.second;
    _logging_rif_info = startup_result.top_level_logging_demultiplexer;

    if (!rif_entry_points.empty())
    {
      if (resource_descriptions)
      {
        std::unordered_map<gspc::resource::Class, unsigned long> class_sizes;

        resource_descriptions->for_each_node
          ( [&class_sizes]
            (Forest<std::uint64_t, Resource>::Node const& r)
            {
              if (class_sizes.count (r.second.resource_class))
              {
                class_sizes[r.second.resource_class]++;
              }
              else
              {
                class_sizes.emplace (r.second.resource_class, 1);
              }
            }
          );

        // create worker_descriptions
        for ( auto const& class_and_size : class_sizes)
        {
          std::vector<std::string> capabilities;

          for ( std::string const& cpb
              : fhg::util::split<std::string, std::string> (class_and_size.first, '+')
              )
          {
            capabilities.emplace_back (cpb);
          }

          worker_description description
            ({ capabilities, class_and_size.second, 0, 0, boost::none, boost::none, boost::none, boost::none});
          _worker_descriptions.emplace_back (description);
        }
      }
      else
      {
        _worker_descriptions = parse_worker_descriptions (topology_description);
      }

      std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
        const failures (add_worker_impl (_worker_descriptions, rif_entry_points, certificates));

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
      ( std::vector<fhg::rif::entry_point> const& entry_points
      , Certificates const& certificates
      )
  {
    return add_worker (_worker_descriptions, entry_points, certificates);
  }

  std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
    scoped_runtime_system::implementation::started_runtime_system::add_worker
      ( std::vector<worker_description> const& worker_descriptions
      , std::vector<fhg::rif::entry_point> const& entry_points
      , Certificates const& certificates
      )
  {
    if (_gpi_socket)
    {
      throw std::logic_error ("add_worker while vmem is in use");
    }

    return add_worker_impl (worker_descriptions, entry_points, certificates);
  }

  std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
    scoped_runtime_system::implementation::started_runtime_system::add_worker_impl
      ( std::vector<worker_description> const& worker_descriptions
      , std::vector<fhg::rif::entry_point> const& entry_points
      , Certificates const& certificates
      )
  {
    std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
      failures;

    for ( worker_description const& description
        : worker_descriptions
        )
    {
      for ( auto const& fail
          : start_workers_for
              ( entry_points
              , _master_agent_name
              , _master_agent_hostinfo
              , description
              , _processes_storage
              , _gpi_socket
              , _app_path
              , _worker_env_copy_variable
              , _worker_env_copy_current
              , _worker_env_copy_file
              , _worker_env_set_variable
              , _installation_path
              , _info_output
              , FHG_UTIL_MAKE_OPTIONAL
                  ( _logging_rif_entry_point && _logging_rif_info
                  , std::make_pair
                      (*_logging_rif_entry_point, _logging_rif_info->pid)
                  )
              , certificates
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

  namespace
  {
    std::vector<fhg::logging::endpoint> extract_default_receivers
      (boost::program_options::variables_map const& vm)
    {
      fhg::util::boost::program_options::require_all_if_one
        (vm, {"log-host", "log-port"});

      std::vector<fhg::logging::endpoint> receivers;
      if (auto const log_host = get_log_host (vm))
      {
        receivers.emplace_back
          (fhg::logging::tcp_endpoint (*log_host, *get_log_port (vm)));
      }
      return receivers;
    }
  }

  scoped_runtime_system::implementation::implementation
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , std::string const& topology_description
    , boost::optional<rifd_entry_points> const& entry_points
    , rifd_entry_point const& master
    , std::ostream& info_output
    , Certificates const& certificates
    , boost::optional<UniqueForest<Resource>> const&
        resource_descriptions
    )
      : _virtual_memory_socket (get_virtual_memory_socket (vm))
      , _virtual_memory_startup_timeout
        ( get_virtual_memory_startup_timeout (vm)
        ? boost::make_optional
          (std::chrono::seconds (get_virtual_memory_startup_timeout (vm).get()))
        : boost::none
        )
      , _started_runtime_system
          ( get_agent_port (vm)
          , !!_virtual_memory_socket
          , _virtual_memory_socket
          , get_application_search_path (vm)
          ? std::vector<boost::filesystem::path> ({boost::filesystem::canonical (get_application_search_path (vm).get())})
          : std::vector<boost::filesystem::path>()
          , get_worker_env_copy_variable (vm).get_value_or ({})
          , get_worker_env_copy_current (vm).get_value_or (false)
          , get_worker_env_copy_file (vm).get_value_or ({})
          , get_worker_env_set_variable (vm).get_value_or ({})
          , installation.gspc_home()
          , _virtual_memory_startup_timeout
          , topology_description
          , get_virtual_memory_port (vm)
          , get_virtual_memory_netdev_id (vm)
          , !entry_points
            ? decltype (entry_points->_->_entry_points) {}
            : entry_points->_->_entry_points
          , master._->_entry_point
          , info_output
          //! \todo User-configurable.
          , master._->_entry_point
          , extract_default_receivers (vm)
          , certificates
          , resource_descriptions
          )
      , _logger()
      , _virtual_memory_api
        ( _virtual_memory_socket
        ? fhg::util::cxx14::make_unique<gpi::pc::client::api_t>
            (_logger, _virtual_memory_socket->string())
        : nullptr
        )
  {
    if (get_log_directory (vm))
    {
      throw std::invalid_argument
        ("--log-directory given but currently not supported");
    }
    if (get_log_level (vm))
    {
      throw std::invalid_argument
        ("--log-level given but currently not supported");
    }
  }
  std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
    scoped_runtime_system::implementation::add_worker
      (rifd_entry_points const& rifd_entry_points, Certificates const& certificates)
  {
    return _started_runtime_system.add_worker
      (rifd_entry_points._->_entry_points, certificates);
  }
  std::unordered_map<fhg::rif::entry_point, std::list<std::exception_ptr>>
    scoped_runtime_system::implementation::add_worker
      ( std::vector<worker_description> const& descriptions
      , rifd_entry_points const& rifd_entry_points
      , Certificates const& certificates
      )
  {
    return _started_runtime_system.add_worker
      (descriptions, rifd_entry_points._->_entry_points, certificates);
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

  std::unordered_map< rifd_entry_point
                    , std::list<std::exception_ptr>
                    , rifd_entry_point_hash
                    >
    scoped_runtime_system::add_worker
      (rifd_entry_points const& rifd_entry_points, Certificates const& certificates)
  {
    return add_worker
      ( _->_started_runtime_system._worker_descriptions
      , rifd_entry_points
      , certificates
      );
  }

  std::unordered_map< rifd_entry_point
                    , std::list<std::exception_ptr>
                    , rifd_entry_point_hash
                    >
    scoped_runtime_system::add_worker
      ( std::vector<worker_description> const& descriptions
      , rifd_entry_points const& rifd_entry_points
      , Certificates const& certificates
      )
  {
    std::unordered_map< fhg::rif::entry_point
                      , std::list<std::exception_ptr>
                      > const result (_->add_worker (descriptions, rifd_entry_points, certificates));
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

  fhg::logging::endpoint scoped_runtime_system::top_level_log_demultiplexer() const
  {
    if (!_->_started_runtime_system._logging_rif_info)
    {
      throw std::logic_error ("No top level log demultiplexer was started.");
    }
    return _->_started_runtime_system._logging_rif_info->sink_endpoint;
  }
}
