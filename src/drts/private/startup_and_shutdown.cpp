#include <drts/private/startup_and_shutdown.hpp>

#include <drts/private/drts_impl.hpp>

#include <util-generic/join.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/read_lines.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/split.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>
#include <fhg/util/starts_with.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <rif/client.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/serialization/unordered_map.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <numeric>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fhg
{
  namespace drts
  {
    void processes_storage::store ( fhg::rif::entry_point const& entry_point
                                  , std::string const& name
                                  , pid_t pid
                                  )
    {
      std::lock_guard<std::mutex> const guard (_guard);

      if (!_[entry_point].emplace (name, pid).second)
      {
        throw std::logic_error
          ( "process with name '" + name + "' on entry point '"
          + entry_point.string() + "' already exists with pid "
          + std::to_string (_.at (entry_point).at (name))
          + ", new pid " + std::to_string (pid)
          );
      }
    }
    boost::optional<pid_t> processes_storage::pidof
      (fhg::rif::entry_point const& entry_point, std::string const& name)
    {
      std::lock_guard<std::mutex> const guard (_guard);

      auto pos_entry_point (_.find (entry_point));

      if (pos_entry_point == _.end())
      {
        return boost::none;
      }

      auto pos_name (pos_entry_point->second.find (name));

      if (pos_name == pos_entry_point->second.end())
      {
        return boost::none;
      }

      return pos_name->second;
    }
  }
}

namespace
{
  std::string build_parent_with_hostinfo
    (std::string const& name, fhg::drts::hostinfo_type const& hostinfo)
  {
    return ( boost::format ("%1%%%%2%%%%3%")
           % name
           % hostinfo.first
           % hostinfo.second
           ).str();
  }


  std::string replace_whitespace (std::string s)
  {
    std::transform ( s.begin(), s.end(), s.begin()
                   , [] (char c)
                     {
                       return std::isspace (c) ? '_' : c;
                     }
                   );
    return s;
  }

  template<typename Res, typename Enum, typename Match>
    boost::optional<Res> get_match (Match& match, Enum part)
  {
    typename Match::const_reference submatch
      (match[static_cast<typename std::underlying_type<Enum>::type> (part)]);
    if (submatch.matched)
    {
      return boost::lexical_cast<Res> (submatch.str());
    }
    return boost::none;
  }

  std::unordered_map<std::string, std::string> logging_environment
    ( boost::optional<std::string> const& log_host
    , boost::optional<unsigned short> const& log_port
    , boost::optional<boost::filesystem::path> const& log_dir
    , bool verbose
    , std::string name
    )
  {
    std::unordered_map<std::string, std::string> environment;
    environment.emplace ("FHGLOG_level", verbose ? "TRACE" : "INFO");
    if (log_host && log_port)
    {
      environment.emplace
        ("FHGLOG_to_server", *log_host + ":" + std::to_string (*log_port));
    }
    if (log_dir)
    {
      environment.emplace
        ( "FHGLOG_to_file"
        , (*log_dir / (replace_whitespace (name) + ".log")).string()
        );
    }
    return environment;
  }

  fhg::drts::hostinfo_type start_agent
    ( fhg::rif::entry_point const& rif_entry_point
    , fhg::rif::client& rif_client
    , std::string const& name
    , std::string const& parent_name
    , fhg::drts::hostinfo_type const& parent_hostinfo
    , boost::optional<std::string> const& gui_host
    , boost::optional<unsigned short> const& gui_port
    , boost::optional<std::string> const& log_host
    , boost::optional<unsigned short> const& log_port
    , boost::optional<boost::filesystem::path> const& gpi_socket
    , bool verbose
    , gspc::installation_path const& installation_path
    , boost::optional<boost::filesystem::path> const& log_dir
    , fhg::drts::processes_storage& processes
    , std::ostream& info_output
    , std::list<fhg::logging::tcp_endpoint>& log_emitters
    )
  {
    info_output << "I: starting agent: " << name << " on rif entry point "
                << rif_entry_point
                << " with parent " << parent_name << "\n";

    auto const result
      ( rif_client.start_agent
          ( name
          , parent_hostinfo
          , gui_host
          , gui_port
          , gpi_socket
          , installation_path.agent()
          , logging_environment (log_host, log_port, log_dir, verbose, name)
          ).get()
      );

    processes.store (rif_entry_point, name, result.pid);

    log_emitters.emplace_back
      (std::move (result.logger_registration_endpoint));

    return result.hostinfo;
  }
}

namespace fhg
{
  namespace drts
  {
    std::pair< std::unordered_set<fhg::rif::entry_point>
             , std::unordered_map<fhg::rif::entry_point, std::exception_ptr>
             > start_workers_for
    ( std::vector<fhg::rif::entry_point> const& entry_points
    , std::string master_name
    , fhg::drts::hostinfo_type master_hostinfo
    , fhg::drts::worker_description const& description
    , bool verbose
    , boost::optional<std::string> const& gui_host
    , boost::optional<unsigned short> const& gui_port
    , boost::optional<std::string> const& log_host
    , boost::optional<unsigned short> const& log_port
    , fhg::drts::processes_storage& processes
    , boost::optional<boost::filesystem::path> const& log_dir
    , boost::optional<boost::filesystem::path> const& gpi_socket
    , std::vector<boost::filesystem::path> const& app_path
    , gspc::installation_path const& installation_path
    , std::ostream& info_output
    )
  {
     std::string name_prefix (fhg::util::join (description.capabilities, '+').string());
     std::replace_if
       (name_prefix.begin(), name_prefix.end(), boost::is_any_of ("+#.-"), '_');

     info_output << "I: starting " << name_prefix << " workers (master "
                 << master_name << ", "
                 << description.num_per_node << "/host, "
                 << ( description.max_nodes == 0 ? "unlimited"
                    : description.max_nodes == 1 ? "unique"
                    : "global max: " + std::to_string (description.max_nodes)
                    )
                 << ", " << description.shm_size << " SHM) with parent "
                 << master_name << " on rif entry point "
                 << fhg::util::join
                      ( entry_points
                      | boost::adaptors::transformed
                          ( [] (fhg::rif::entry_point const& entry_point)
                            {
                              return entry_point.string();
                            }
                          )
                      , ", "
                      )
                 << "\n";

     std::vector<std::string> arguments;

     arguments.emplace_back ("--master");
     arguments.emplace_back
       (build_parent_with_hostinfo (master_name, master_hostinfo));

     //! \todo gui is optional in worker
     if (gui_host && gui_port)
     {
       arguments.emplace_back ("--gui-host");
       arguments.emplace_back (*gui_host);
       arguments.emplace_back ("--gui-port");
       arguments.emplace_back (std::to_string (*gui_port));
     }

     for (boost::filesystem::path const& path : app_path)
     {
       arguments.emplace_back ("--library-search-path");
       arguments.emplace_back (path.string());
     }

     if (description.shm_size)
     {
       arguments.emplace_back ("--capability");
       arguments.emplace_back ("GPI");
       arguments.emplace_back ("--virtual-memory-socket");
       arguments.emplace_back (gpi_socket.get().string());
       arguments.emplace_back ("--shared-memory-size");
       arguments.emplace_back (std::to_string (description.shm_size));
     }

     for (std::string const& capability : description.capabilities)
     {
       arguments.emplace_back ("--capability");
       arguments.emplace_back (capability);
     }

     if (description.socket)
     {
       arguments.emplace_back ("--socket");
       arguments.emplace_back (std::to_string (description.socket.get()));
     }

     std::atomic<std::size_t> num_nodes (0);

      std::unordered_map<fhg::rif::entry_point, std::vector<std::exception_ptr>>
        exceptions;

      //! \todo let thread count be a parameter
      fhg::util::scoped_boost_asio_io_service_with_threads io_service
        (std::min (64UL, entry_points.size()));

      std::list<std::pair<rif::client, rif::entry_point>> rif_connections;
      util::nest_exceptions<std::runtime_error>
        ( [&]
          {
            for (rif::entry_point const& entry_point : entry_points)
            {
              rif_connections.emplace_back
                ( std::piecewise_construct
                , std::forward_as_tuple (io_service, entry_point)
                , std::forward_as_tuple (entry_point)
                );
            }
          }
        , "connecting to rif entry points"
        );

      std::vector<std::tuple< fhg::rif::entry_point
                            , std::future<fhg::rif::protocol::start_worker_result>
                            , std::string
                            >
                 > futures;

      for (auto& connection : rif_connections)
      {
        //! \todo does this work correctly for multi-segments?!
        if ( description.max_nodes != 0
           && num_nodes.fetch_add (1) >= description.max_nodes
           )
        {
          break;
        }

        for ( unsigned long identity (0)
            ; identity < description.num_per_node
            ; ++identity
            )
        {
          try
          {
            std::string const name
              ( name_prefix + "-" + connection.second.string()
              + "-" + std::to_string (identity + 1)
              + ( description.socket
                ? ("." + std::to_string (description.socket.get()))
                : std::string()
                )
              );
            std::string const storage_name ("drts-kernel-" + name);

            {
              boost::optional<pid_t> const mpid
                (processes.pidof (connection.second, storage_name));

              if (!!mpid)
              {
                throw std::logic_error
                  ( "process with name '" + name + "' on entry point '"
                  + connection.second.string() + "' already exists with pid "
                  + std::to_string (*mpid)
                  );
              }
            }

            std::unordered_map<std::string, std::string> environment
              ( logging_environment
                  (log_host, log_port, log_dir, verbose, name)
              );
            environment.emplace
              ( "LD_LIBRARY_PATH"
              , (installation_path.lib()).string() + ":"
              + (installation_path.libexec()).string()
              );

            futures.emplace_back
              ( connection.second
              , connection.first.start_worker
                  ( name
                  , installation_path.drts_kernel()
                  , arguments
                  , environment
                  )
              , name
              );
          }
          catch (...)
          {
            exceptions[connection.second].emplace_back
              (std::current_exception());
          }
        }
      }

      for (auto& future : futures)
      {
        try
        {
          auto const result (std::get<1> (future).get());

          processes.store ( std::get<0> (future)
                          , "drts-kernel-" + std::get<2> (future)
                          , result.pid
                          );
        }
        catch (...)
        {
          exceptions[std::get<0> (future)].emplace_back
            (std::current_exception());
        }
      }

      std::pair< std::unordered_set<fhg::rif::entry_point>
               , std::unordered_map<fhg::rif::entry_point, std::exception_ptr>
               > results;

      for (auto const& connection : rif_connections)
      {
        try
        {
          auto const it (exceptions.find ((connection.second)));
          if (it == exceptions.end())
          {
            results.first.emplace (connection.second);
          }
          else
          {
            //! \todo return the individual exceptions
            fhg::util::throw_collected_exceptions (it->second);
          }
        }
        catch (...)
        {
          results.second.emplace (connection.second, std::current_exception());
        }
      }

      return results;
    }

    worker_description parse_capability
      (std::size_t def_num_proc, std::string const& cap_spec)
    {
      static std::regex const cap_spec_regex
        ("^([^#:]+)(#([0-9]+))?(:([0-9]+)(x([0-9]+))?(,([0-9]+))?)?$");
      enum class cap_spec_regex_part
      {
        capabilities = 1,
        socket = 3,
        num_per_node = 5,
        max_nodes = 7,
        shm = 9,
      };

      std::smatch cap_spec_match;
      if (!std::regex_match (cap_spec, cap_spec_match, cap_spec_regex))
      {
        throw std::invalid_argument
          ("Invalid capability specification: " + cap_spec);
      }

      std::size_t const num_per_node
        ( get_match<std::size_t>
            (cap_spec_match, cap_spec_regex_part::num_per_node)
          .get_value_or (def_num_proc)
        );

      if (num_per_node == 0)
      {
        throw std::invalid_argument
          ( "invalid number of workers per node in capability specification: "
            "positive integer expected in " + cap_spec
          );
      }

      return
        { fhg::util::split<std::string, std::string, std::vector<std::string>>
            ( get_match<std::string>
                (cap_spec_match, cap_spec_regex_part::capabilities).get()
            , '+'
            )
        , num_per_node
        , get_match<std::size_t> (cap_spec_match, cap_spec_regex_part::max_nodes)
          .get_value_or (0)
        , get_match<std::size_t> (cap_spec_match, cap_spec_regex_part::shm)
          .get_value_or (0)
        , get_match<std::size_t> (cap_spec_match, cap_spec_regex_part::socket)
        };
    }

    hostinfo_type startup
      ( boost::optional<std::string> const& gui_host
      , boost::optional<unsigned short> const& gui_port
      , boost::optional<std::string> const& log_host
      , boost::optional<unsigned short> const& log_port
      , bool gpi_enabled
      , bool verbose
      , boost::optional<boost::filesystem::path> gpi_socket
      , gspc::installation_path const& installation_path
      , bool delete_logfiles
      , fhg::util::signal_handler_manager& signal_handler_manager
      , boost::optional<std::chrono::seconds> vmem_startup_timeout
      , boost::optional<unsigned short> vmem_port
      , std::vector<fhg::rif::entry_point> const& rif_entry_points
      , fhg::rif::entry_point const& master
      , boost::optional<boost::filesystem::path> const& log_dir
      , fhg::drts::processes_storage& processes
      , std::string& master_agent_name
      , fhg::drts::hostinfo_type& master_agent_hostinfo
      , std::ostream& info_output
      , std::list<fhg::logging::tcp_endpoint>& log_emitters
      )
    {
      if (log_dir)
      {
        if (delete_logfiles)
        {
          boost::filesystem::remove_all (*log_dir);
        }
        boost::filesystem::create_directories (*log_dir);
      }

      fhg::util::scoped_signal_handler interrupt_signal_handler
        ( signal_handler_manager
        ,  SIGINT
        , [] (int, siginfo_t*, void*)
          {
            throw std::runtime_error ("Canceled...");
          }
        );

      info_output << "I: starting base sdpa components on " << master << "...\n";
      if (log_host && log_port)
      {
        info_output << "I: sending log events to: "
                    << *log_host << ":" << *log_port << "\n";
      }
      if (gui_host && gui_port)
      {
        info_output << "I: sending execution events to: "
                    << *gui_host << ":" << *gui_port << "\n";
      }

      //! \todo let thread count be a parameter
      fhg::util::scoped_boost_asio_io_service_with_threads io_service
        (std::max (1UL, std::min (64UL, rif_entry_points.size())));

      rif::client master_rif_client (io_service, master);

      std::pair<pid_t, std::vector<std::string>> const orchestrator_startup_messages
        ( fhg::util::nest_exceptions<std::runtime_error>
            ( [&]
              {
                return master_rif_client.execute_and_get_startup_messages
                  ( installation_path.orchestrator()
                  , std::vector<std::string> {"-u", "0", "-n", "orchestrator"}
                  , logging_environment
                      (log_host, log_port, log_dir, verbose, "orchestrator")
                  ).get();
              }
              , "could not start orchestrator"
              )
        );

      if (orchestrator_startup_messages.second.size() != 2)
      {
        throw std::logic_error
          ("could not start orchestrator: expected 2 lines of startup messages");
      }

      processes.store
        (master, "orchestrator", orchestrator_startup_messages.first);

      hostinfo_type const orchestrator_hostinfo
        ( orchestrator_startup_messages.second[0]
        , boost::lexical_cast<unsigned short>
            (orchestrator_startup_messages.second[1])
        );

      std::list<std::pair<rif::client, rif::entry_point>> rif_connections;
      std::vector<std::string> hostnames;
      util::nest_exceptions<std::runtime_error>
        ( [&]
          {
            for (rif::entry_point const& entry_point : rif_entry_points)
            {
              rif_connections.emplace_back
                ( std::piecewise_construct
                , std::forward_as_tuple (io_service, entry_point)
                , std::forward_as_tuple (entry_point)
                );
              hostnames.emplace_back (entry_point.hostname);
            }
          }
        , "connecting to rif entry points"
        );

      if (gpi_enabled)
      {
        if (!vmem_startup_timeout)
        {
          throw std::invalid_argument
            ("vmem startup timeout is required when gpi is enabled");
        }
        if (!vmem_port)
        {
          throw std::invalid_argument
            ("vmem port is required when gpi is enabled");
        }
        if (!gpi_socket)
        {
          throw std::invalid_argument
            ("vmem socket is required when gpi is enabled");
        }

        info_output << "I: starting VMEM on: " << gpi_socket.get()
                    << " with a timeout of " << vmem_startup_timeout.get().count()
                    << " seconds\n";

        fhg::util::nest_exceptions<std::runtime_error>
          ( [&]
            {
              std::unordered_map<fhg::rif::entry_point, std::future<pid_t>>
                queued_start_requests;
              std::unordered_map<fhg::rif::entry_point, std::exception_ptr>
                fails;

              //! \note requires ranks to be matching index in hostnames!
              std::size_t rank (0);
              for (auto& connection : rif_connections)
              {
                try
                {
                  queued_start_requests.emplace
                    ( connection.second
                    , connection.first.start_vmem
                        ( installation_path.vmem()
                        , verbose ? fhg::log::TRACE : fhg::log::INFO
                        , gpi_socket.get()
                        , vmem_port.get()
                        , vmem_startup_timeout.get()
                        , log_host && log_port
                        ? std::make_pair (log_host.get(), log_port.get())
                        : boost::optional<std::pair<std::string, unsigned short>>()
                        , log_dir
                        ? *log_dir / ("vmem-" + replace_whitespace (connection.second.string()) + ".log")
                        : boost::optional<boost::filesystem::path>()
                        , hostnames
                        , master.string()
                        , rank++
                        )
                    );
                }
                catch (...)
                {
                  fails.emplace (connection.second, std::current_exception());
                }
              }

              for (auto& request : queued_start_requests)
              {
                try
                {
                  processes.store (request.first, "vmem", request.second.get());
                }
                catch (...)
                {
                  fails.emplace (request.first, std::current_exception());
                }
              }

              if (!fails.empty())
              {
                fhg::util::throw_collected_exceptions
                  ( fails
                  , [] (std::pair<rif::entry_point, std::exception_ptr> const& fail)
                    {
                      return ( boost::format ("vmem startup failed %1%: %2%")
                             % fail.first
                             % fhg::util::exception_printer (fail.second)
                             ).str();
                    }
                  );
              }
            }
          , "could not start vmem"
          );
      }

      master_agent_name = "agent-" + master.string() + "-0";

      master_agent_hostinfo = start_agent ( master
                                          , master_rif_client
                                          , master_agent_name
                                          , "orchestrator"
                                          , orchestrator_hostinfo
                                          , gui_host
                                          , gui_port
                                          , log_host
                                          , log_port
                                          , gpi_socket
                                          , verbose
                                          , installation_path
                                          , log_dir
                                          , processes
                                          , info_output
                                          , log_emitters
                                          );

      return orchestrator_hostinfo;
    }

    namespace
    {
      template<typename It>
        std::unordered_map
          < rif::entry_point
          , std::pair< std::string /* kind */
                     , std::unordered_map<pid_t, std::exception_ptr>
                     >
          > terminate_all_processes_of_a_kind
              ( std::vector<It> const& entry_point_procs
              , component_type component
              , std::ostream& info_output
              )
      {
        std::string const kind
          ( component == component_type::worker ? "drts-kernel"
          : component == component_type::agent ? "agent"
          : component == component_type::orchestrator ? "orchestrator"
          : component == component_type::vmem ? "vmem"
          : throw std::logic_error ("invalid enum value")
          );

        std::unordered_map
          < rif::entry_point
          , std::pair< std::string /* kind */
                     , std::unordered_map<pid_t, std::exception_ptr>
                     >
          > failures;

        std::size_t processes_to_kill (0);
        for (auto const& entry_point_processes : entry_point_procs)
        {
          for (auto const& it : entry_point_processes->second)
          {
            if (fhg::util::starts_with (kind, it.first))
            {
              ++processes_to_kill;
            }
          }
        }

        //! \todo let thread count be a parameter
        fhg::util::scoped_boost_asio_io_service_with_threads io_service
          (std::min (64UL, processes_to_kill));

        using process_iter
          = typename decltype (entry_point_procs.front()->second)::iterator;

        std::list<fhg::rif::client> clients;
        std::vector<std::tuple < It
                               , std::future<std::unordered_map<pid_t, std::exception_ptr>>
                               , std::unordered_map<pid_t, process_iter>
                               , std::function<void()>
                               >
                   > futures;

        for (auto const& entry_point_processes : entry_point_procs)
        {
          std::vector<pid_t> pids;
          std::unordered_map<pid_t, process_iter> to_erase;
          for ( process_iter it (entry_point_processes->second.begin())
              ; it != entry_point_processes->second.end()
              ; ++it
              )
          {
            if (fhg::util::starts_with (kind, it->first))
            {
              to_erase.emplace (it->second, it);
              pids.emplace_back (it->second);
            }
          }

          auto const& entry_point (entry_point_processes->first);
          auto&& fail_group_with_current_exception
            ( [&failures, pids, entry_point, &kind]
              {
                std::unordered_map<pid_t, std::exception_ptr> fails;

                for (pid_t pid : pids)
                {
                  fails.emplace (pid, std::current_exception());
                }

                failures.emplace ( entry_point
                                 , std::make_pair (kind, fails)
                                 );
              }
            );

          if (!pids.empty())
          {
            info_output << "terminating " << kind << " on "
                        << entry_point
                        << ": " << fhg::util::join (pids, ' ') << "\n";

            try
            {
              clients.emplace_back (io_service, entry_point);
              futures.emplace_back ( entry_point_processes
                                   , clients.back().kill (pids)
                                   , std::move (to_erase)
                                   , fail_group_with_current_exception
                                   );
            }
            catch (...) // \note: e.g. rif::client::connect
            {
              fail_group_with_current_exception();
            }
          }
        }

        for (auto& future : futures)
        {
          try
          {
            auto const failures_kill (std::get<1> (future).get());

            if (!failures_kill.empty())
            {
              failures.emplace
                ( std::get<0> (future)->first
                , std::make_pair (kind, failures_kill)
                );
            }
          }
          catch (...)
          {
            std::get<3> (future)();
          }

          //! \note: remove the process from the list of known
          //! processes in case of failure too assumption: when kill
          //! failed once, it will never succeed
          for (auto const& iter : std::get<2> (future))
          {
            std::get<0> (future)->second.erase (iter.second);
          }
        }

        return failures;
      }
    }

    std::unordered_map< rif::entry_point
                      , std::pair< std::string /* kind */
                                 , std::unordered_map<pid_t, std::exception_ptr>
                                 >
                      >
      processes_storage::shutdown_worker
        (std::vector<fhg::rif::entry_point> const& rif_entry_points)
    {
      std::vector<decltype (_)::iterator> iterators;
      for (fhg::rif::entry_point const& entry_point : rif_entry_points)
      {
        decltype (_)::iterator const pos (_.find (entry_point));
        if (pos == _.end())
        {
          throw std::invalid_argument ("shutdown for unknown entry_point");
        }
        //! \todo detect if there are any components on this entry_point
        iterators.emplace_back (pos);
      }

      std::unordered_map
        < rif::entry_point
        , std::pair< std::string /* kind */
                   , std::unordered_map<pid_t, std::exception_ptr>
                   >
        > const failures
            (terminate_all_processes_of_a_kind ( iterators
                                               , component_type::worker
                                               , _info_output
                                               )
            );

      for (decltype (_)::iterator const& it : iterators)
      {
        if (it->second.empty())
        {
          _.erase (it);
        }
      }

      return failures;
    }

    namespace
    {
      template<typename It>
        std::vector<It> iterators (It begin, It end)
      {
        std::vector<It> res (std::distance (begin, end));
        std::iota (res.begin(), res.end(), begin);
        return res;
      }
    }

    processes_storage::~processes_storage()
    {
      util::apply_for_each_and_collect_exceptions
        ( { component_type::worker
          , component_type::agent
          , component_type::vmem
          , component_type::orchestrator
          }
        , [this] (component_type component)
          {
            std::unordered_map
              < rif::entry_point
              , std::pair< std::string /* kind */
                         , std::unordered_map<pid_t, std::exception_ptr>
                         >
              > const failures
                  ( terminate_all_processes_of_a_kind
                      (iterators (_.begin(), _.end()), component, _info_output)
                  );

            for (auto const& failure : failures)
            {
              for (auto const& fails : failure.second.second)
              {
                _info_output <<
                  ( boost::format ("Could not terminate %1%[%2%] on %3%: %4%")
                  % failure.second.first
                  % fails.first
                  % failure.first
                  % util::exception_printer (fails.second)
                  ) << std::endl;
              }
            }
          }
        );
    }
  }
}
