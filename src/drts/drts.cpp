// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <drts/drts.hpp>
#include <drts/private/drts_impl.hpp>
#include <drts/private/option.hpp>
#include <drts/private/pimpl.hpp>
#include <drts/private/rifd_entry_points_impl.hpp>
#include <drts/private/startup_and_shutdown.hpp>
#include <drts/drts_iml.hpp>

#include <we/type/Activity.hpp>
#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>

#include <iml/Client.hpp>
#include <iml/RuntimeSystem.hpp>

#include <sdpa/client.hpp>

#include <fhg/project_version.hpp>
#include <util-generic/boost/program_options/require_all_if_one.hpp>
#include <util-generic/make_optional.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/read_lines.hpp>
#include <util-generic/split.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <boost/format.hpp>

#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace gspc
{
  installation::installation
    (::boost::filesystem::path const& gspc_home)
      : _gspc_home (::boost::filesystem::canonical (gspc_home))
  {
    ::boost::filesystem::path const path_version (_gspc_home / "version");

    if (!::boost::filesystem::exists (path_version))
    {
      throw std::invalid_argument
        (( ::boost::format ("GSPC version mismatch: File '%1%' does not exist.")
         % path_version
         ).str());
    }

    std::string const version (fhg::util::read_file (path_version));

    if (version != fhg::project_version())
    {
      throw std::invalid_argument
        (( ::boost::format ( "GSPC version mismatch: Expected '%1%'"
                         ", installation in '%2%' has version '%3%'"
                         )
         % fhg::project_version()
         % _gspc_home
         % version
         ).str()
        );
    }
  }
  installation::installation
    (::boost::program_options::variables_map const& vm)
      : installation (require_gspc_home (vm))
  {}

  scoped_runtime_system::scoped_runtime_system
      ( ::boost::program_options::variables_map const& vm
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
    ( ::boost::program_options::variables_map const& vm
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
    ( ::boost::program_options::variables_map const& vm
    , installation const& installation
    , std::string const& topology_description
    , ::boost::optional<rifd_entry_points> const& entry_points
    , rifd_entry_point const& parent
    , std::ostream& info_output
    , Certificates const& certificates
    )
      : _ (new implementation ( vm
                              , installation
                              , topology_description
                              , entry_points
                              , parent
                              , info_output
                              , certificates
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
        worker_descriptions.emplace_back (description);
      }
      return worker_descriptions;
    }
  }

  scoped_runtime_system::implementation::started_runtime_system::started_runtime_system
      ( ::boost::optional<unsigned short> const& agent_port
      , ::boost::optional<::boost::filesystem::path> gpi_socket
      , std::vector<::boost::filesystem::path> app_path
      , std::vector<std::string> worker_env_copy_variable
      , bool worker_env_copy_current
      , std::vector<::boost::filesystem::path> worker_env_copy_file
      , std::vector<std::string> worker_env_set_variable
      , gspc::installation_path installation_path
      , std::vector<worker_description> worker_descriptions
      , std::vector<fhg::rif::entry_point> const& rif_entry_points
      , fhg::rif::entry_point const& parent
      , std::ostream& info_output
      , ::boost::optional<fhg::rif::entry_point> logging_rif_entry_point
      , std::vector<fhg::logging::endpoint> default_log_receivers
      , Certificates const& certificates
      )
    : _info_output (info_output)
    , _parent (parent)
    , _gpi_socket (gpi_socket)
    , _app_path (app_path)
    , _worker_env_copy_variable (std::move (worker_env_copy_variable))
    , _worker_env_copy_current (worker_env_copy_current)
    , _worker_env_copy_file (std::move (worker_env_copy_file))
    , _worker_env_set_variable (std::move (worker_env_set_variable))
    , _installation_path (installation_path)
    , _logging_rif_entry_point (logging_rif_entry_point)
    , _worker_descriptions (std::move (worker_descriptions))
    , _processes_storage (_info_output)
  {
    fhg::util::signal_handler_manager signal_handler_manager;

    auto const startup_result
      ( fhg::drts::startup
          ( agent_port
          , _gpi_socket
          , _installation_path
          , signal_handler_manager
          , rif_entry_points
          , _parent
          , _processes_storage
          , _parent_agent_name
          , _parent_agent_hostinfo
          , _info_output
          , _logging_rif_entry_point
          , default_log_receivers
          , certificates
          )
      );
    _top_level_agent_host = startup_result.top_level_agent.first;
    _top_level_agent_port = startup_result.top_level_agent.second;
    _logging_rif_info = startup_result.top_level_logging_demultiplexer;

    if (!rif_entry_points.empty())
    {
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
              , _parent_agent_name
              , _parent_agent_hostinfo
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
      (::boost::program_options::variables_map const& vm)
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
    ( ::boost::program_options::variables_map const& vm
    , installation const& installation
    , std::string const& topology_description
    , ::boost::optional<rifd_entry_points> const& entry_points
    , rifd_entry_point const& parent
    , std::ostream& info_output
    , Certificates const& certificates
    )
      : _virtual_memory_socket
        ( get_virtual_memory_socket (vm)
        ? get_virtual_memory_socket (vm)
        : get_remote_iml_vmem_socket (vm)
        )
      , _iml_rts ( FHG_UTIL_MAKE_OPTIONAL
                     ( !!get_virtual_memory_socket (vm)
                     , iml_runtime_system {vm, info_output}
                     )
                 )
      , _started_runtime_system
          ( get_agent_port (vm)
          , _virtual_memory_socket
          , get_application_search_path (vm)
          ? std::vector<::boost::filesystem::path>
            ({::boost::filesystem::canonical (get_application_search_path (vm).get())})
          : std::vector<::boost::filesystem::path>()
          , get_worker_env_copy_variable (vm).get_value_or ({})
          , get_worker_env_copy_current (vm).get_value_or (false)
          , get_worker_env_copy_file (vm).get_value_or ({})
          , get_worker_env_set_variable (vm).get_value_or ({})
          , installation.gspc_home()
          , parse_worker_descriptions (topology_description)
          , !entry_points
            ? decltype (entry_points->_->_entry_points) {}
            : entry_points->_->_entry_points
          , parent._->_entry_point
          , info_output
          //! \todo User-configurable.
          , parent._->_entry_point
          , extract_default_receivers (vm)
          , certificates
          )
      , _logger()
      , _virtual_memory_api
        ( _virtual_memory_socket
        ? std::make_unique<iml::Client> (*_virtual_memory_socket)
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
    if (get_virtual_memory_socket (vm) && get_remote_iml_vmem_socket (vm))
    {
      throw std::invalid_argument
        ( "--virtual-memory-socket and --remote-iml-vmem-socket can't be "
          "given at the same time"
        );
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
    , std::string const&
    ) const
  {
    return iml::SegmentAndAllocation
      {*_->_virtual_memory_api, segment_description, size};
  }
  vmem_allocation scoped_runtime_system::alloc_and_fill
    ( vmem::segment_description segment_description
    , unsigned long size
    , std::string const&
    , char const* const data
    ) const
  {
    return iml::SegmentAndAllocation
      {*_->_virtual_memory_api, segment_description, size, data};
  }
  stream scoped_runtime_system::create_stream
    ( std::string const&
    , gspc::vmem_allocation const& buffer
    , iml::MemorySize size_of_slot
    , std::function<void (::pnet::type::value::value_type const&)> on_slot_filled
    ) const
  {
    return gspc::stream
      ( *_->_virtual_memory_api
      , buffer.iml_allocation()
      , size_of_slot
      , std::move (on_slot_filled)
      );
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

  namespace
  {
    std::unique_ptr<iml::Rifs> make_iml_scoped_rifds
      (::boost::program_options::variables_map vm)
    {
      return std::make_unique<iml::Rifs>
        ( fhg::util::read_lines (require_nodefile (vm))
        , require_rif_strategy (vm)
        , require_rif_strategy_parameters (vm)
        , get_rif_port (vm)
        );
    }

    std::unique_ptr<iml::RuntimeSystem> make_iml_rts
      ( ::boost::program_options::variables_map const& vm
      , std::ostream& info_output
      , iml::Rifs const& rifds
      )
    {
      return std::make_unique<iml::RuntimeSystem>
        ( rifds
        , require_virtual_memory_socket (vm)
        , require_virtual_memory_port (vm)
        , std::chrono::seconds (require_virtual_memory_startup_timeout (vm))
        , require_virtual_memory_netdev_id (vm)
        , info_output
        );
    }
  }

  iml_runtime_system::iml_runtime_system
      ( ::boost::program_options::variables_map const& vm
      , std::ostream& info_output
      )
    : rifds (make_iml_scoped_rifds (vm))
    , rts (make_iml_rts (vm, info_output, *rifds))
  {}
}
