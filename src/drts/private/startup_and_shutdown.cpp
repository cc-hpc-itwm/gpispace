#include <drts/private/startup_and_shutdown.hpp>

#include <drts/private/drts_impl.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/nest_exceptions.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/read_lines.hpp>
#include <util-generic/split.hpp>
#include <fhg/util/starts_with.hpp>
#include <fhg/util/wait_and_collect_exceptions.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>
#include <util-generic/serialization/std/unordered_map.hpp>

#include <rif/client.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/regex.hpp>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
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
      if (!_[entry_point].emplace (name, pid).second)
      {
        throw std::logic_error
          ( "process with name '" + name + "' on entry point '"
          + entry_point.to_string() + "' already exists with pid "
          + std::to_string (_.at (entry_point).at (name))
          + ", new pid " + std::to_string (pid)
          );
      }
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
    , std::string const& name
    , std::string const& parent_name
    , fhg::drts::hostinfo_type const& parent_hostinfo
    , boost::optional<std::string> const& gui_host
    , boost::optional<unsigned short> const& gui_port
    , boost::optional<std::string> const& log_host
    , boost::optional<unsigned short> const& log_port
    , boost::optional<boost::filesystem::path> const& gpi_socket
    , bool verbose
    , boost::filesystem::path const& sdpa_home
    , boost::optional<boost::filesystem::path> const& log_dir
    , fhg::drts::processes_storage& processes
    )
  {
    std::cout << "I: starting agent: " << name << " on rif entry point "
              << rif_entry_point.to_string()
              << " with parent " << parent_name << "\n";

    std::vector<std::string> agent_startup_arguments
      { "-u", "0"
      , "-n", name
      , "-m", build_parent_with_hostinfo (parent_name, parent_hostinfo)
      };
    if (gui_host && gui_port)
    {
      agent_startup_arguments.emplace_back ("-a");
      agent_startup_arguments.emplace_back
        (*gui_host + ":" + std::to_string (*gui_port));
    }
    if (gpi_socket)
    {
      agent_startup_arguments.emplace_back ("--vmem-socket");
      agent_startup_arguments.emplace_back (gpi_socket->string());
    }

    std::pair<pid_t, std::vector<std::string>> const agent_startup_messages
      ( fhg::rif::client (rif_entry_point).execute_and_get_startup_messages
          ( sdpa_home / "bin" / "agent"
          , agent_startup_arguments
          , logging_environment (log_host, log_port, log_dir, verbose, name)
          ).get()
      );

    if (agent_startup_messages.second.size() != 2)
    {
      throw std::logic_error ( "could not start agent " + name
                             + ": expected 2 lines of startup messages"
                             );
    }

    processes.store (rif_entry_point, name, agent_startup_messages.first);

    return { agent_startup_messages.second[0]
           , boost::lexical_cast<unsigned short> (agent_startup_messages.second[1])
           };
  }
}

namespace fhg
{
  namespace drts
  {
  void start_workers_for
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
    , boost::filesystem::path const& sdpa_home
    )
  {
     std::string name_prefix (fhg::util::join (description.capabilities, "+"));
     std::replace_if
       (name_prefix.begin(), name_prefix.end(), boost::is_any_of ("+#.-"), '_');

     std::cout << "I: starting " << name_prefix << " workers (master "
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
                            return entry_point.to_string();
                          }
                        )
                    , ", "
                    )
               << "\n";

     std::vector<std::future<void>> startups;

     std::size_t num_nodes (0);
     for (fhg::rif::entry_point const& entry_point : entry_points)
     {
       for ( unsigned long identity (0)
           ; identity < description.num_per_node
           ; ++identity
           )
       {
         startups.emplace_back
           ( std::async
               ( std::launch::async
               , [&, entry_point, identity]
                 {
                   std::vector<std::string> arguments;

                   arguments.emplace_back ("--master");
                   arguments.emplace_back
                     ( build_parent_with_hostinfo
                         (master_name, master_hostinfo)
                     );

                   arguments.emplace_back ("--backlog-length");
                   arguments.emplace_back ("1");

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
                     arguments.emplace_back
                       (std::to_string (description.shm_size));
                   }

                   for (std::string const& capability : description.capabilities)
                   {
                     arguments.emplace_back ("--capability");
                     arguments.emplace_back (capability);
                   }

                   if (description.socket)
                   {
                     arguments.emplace_back ("--socket");
                     arguments.emplace_back
                       (std::to_string (description.socket.get()));
                   }

                   std::string const name
                     ( name_prefix + "-" + entry_point.to_string()
                     + "-" + std::to_string (identity + 1)
                     + ( description.socket
                       ? ("." + std::to_string (description.socket.get()))
                       : std::string()
                       )
                     );

                   arguments.emplace_back ("-n");
                   arguments.emplace_back (name);

                   std::unordered_map<std::string, std::string> environment
                     ( logging_environment
                         (log_host, log_port, log_dir, verbose, name)
                     );
                   environment.emplace ( "LD_LIBRARY_PATH"
                                       , (sdpa_home / "lib").string() + ":"
                                       + (sdpa_home / "libexec" / "sdpa").string()
                                       );

                   std::pair<pid_t, std::vector<std::string>> const pid_and_startup_messages
                     ( fhg::rif::client (entry_point).execute_and_get_startup_messages
                       ( sdpa_home / "bin" / "drts-kernel"
                       , arguments
                       , environment
                       ).get()
                     );

                   if (!pid_and_startup_messages.second.empty())
                   {
                     throw std::runtime_error
                       ("could not start " + name + ": expected no startup messages");
                   }

                   processes.store ( entry_point
                                   , "drts-kernel-" + name
                                   , pid_and_startup_messages.first
                                   );
                 }
               )
           );
       }

       //! \todo does this work correctly for multi-segments?!
       ++num_nodes;
       if (num_nodes == description.max_nodes)
       {
         break;
       }
     }

     fhg::util::wait_and_collect_exceptions (startups);
  }

    worker_description parse_capability
      (std::size_t def_num_proc, std::string const& cap_spec)
    {
      static boost::regex const cap_spec_regex
        ("^([^#:]+)(#([0-9]+))?(:([0-9]+)(x([0-9]+))?(,([0-9]+))?)?$");
      enum class cap_spec_regex_part
      {
        capabilities = 1,
        socket = 3,
        num_per_node = 5,
        max_nodes = 7,
        shm = 9,
      };

      boost::smatch cap_spec_match;
      if (!boost::regex_match (cap_spec, cap_spec_match, cap_spec_regex))
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
      , boost::filesystem::path sdpa_home
      , bool delete_logfiles
      , fhg::util::signal_handler_manager& signal_handler_manager
      , boost::optional<std::size_t> gpi_mem
      , boost::optional<std::chrono::seconds> vmem_startup_timeout
      , boost::optional<unsigned short> vmem_port
      , std::vector<fhg::rif::entry_point> const& rif_entry_points
      , boost::optional<boost::filesystem::path> const& log_dir
      , fhg::drts::processes_storage& processes
      , std::string& master_agent_name
      , fhg::drts::hostinfo_type& master_agent_hostinfo
      )
    {
      if (rif_entry_points.empty())
      {
        throw std::invalid_argument ("rif_entry_points empty");
      }

      fhg::rif::entry_point const& master (rif_entry_points.front());

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

      std::cout << "I: starting base sdpa components on " << master.to_string() << "...\n";
      if (log_host && log_port)
      {
        std::cout << "I: sending log events to: "
                  << *log_host << ":" << *log_port << "\n";
      }
      if (gui_host && gui_port)
      {
        std::cout << "I: sending execution events to: "
                  << *gui_host << ":" << *gui_port << "\n";
      }

      std::pair<pid_t, std::vector<std::string>> const orchestrator_startup_messages
        ( fhg::util::nest_exceptions<std::runtime_error>
            ( [&]
              {
                return rif::client (master).execute_and_get_startup_messages
                  ( sdpa_home / "bin" / "orchestrator"
                  , {"-u", "0", "-n", "orchestrator"}
                  , logging_environment
                      (log_host, log_port, log_dir, verbose, "orchestrator")
                  ).get();
              }
              , "could not start orchestrator"
              )
        );

      if (orchestrator_startup_messages.second.size() != 4)
      {
        throw std::logic_error
          ("could not start orchestrator: expected 4 lines of startup messages");
      }

      processes.store
        (master, "orchestrator", orchestrator_startup_messages.first);

      hostinfo_type const orchestrator_hostinfo
        ( orchestrator_startup_messages.second[0]
        , boost::lexical_cast<unsigned short>
            (orchestrator_startup_messages.second[1])
        );
      hostinfo_type const orchestrator_rpc_hostinfo
        ( orchestrator_startup_messages.second[2]
        , boost::lexical_cast<unsigned short>
            (orchestrator_startup_messages.second[3])
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
        if (!gpi_mem)
        {
          throw std::invalid_argument
            ("vmem memory size is required when gpi is enabled");
        }

        std::cout << "I: using VMEM mem: " << gpi_mem.get() << " bytes per node\n"
                  << "I: starting VMEM on: " << gpi_socket.get()
                  << " with a timeout of " << vmem_startup_timeout.get().count()
                  << " seconds\n";

        std::vector<std::string> nodes;
        for (fhg::rif::entry_point const& entry_point : rif_entry_points)
        {
          nodes.emplace_back (entry_point.hostname);
        }

        fhg::util::nest_exceptions<std::runtime_error>
          ( [&]
            {
              std::vector<std::future<void>> futures;

              for (fhg::rif::entry_point const& entry_point : rif_entry_points)
              {
                futures.emplace_back
                  ( std::async
                      ( std::launch::async
                      , [&, entry_point]
                      {
                        pid_t const pid
                          ( rif::client (entry_point).start_vmem
                              ( sdpa_home / "bin" / "gpi-space"
                              , verbose ? fhg::log::TRACE : fhg::log::INFO
                              , gpi_mem.get()
                              , gpi_socket.get()
                              , vmem_port.get()
                              , vmem_startup_timeout.get()
                              , rif_entry_points.size() > 1 ? "gaspi" : "fake"
                              , log_host && log_port
                              ? std::make_pair (log_host.get(), log_port.get())
                              : boost::optional<std::pair<std::string, unsigned short>>()
                              , log_dir
                              ? *log_dir / ("vmem-" + replace_whitespace (entry_point.to_string()) + ".log")
                              : boost::optional<boost::filesystem::path>()
                              , nodes
                              , master.to_string()
                              , entry_point == master
                              ).get()
                          );

                        processes.store (entry_point, "vmem", pid);
                      }
                    )
                );
              }

              fhg::util::wait_and_collect_exceptions (futures);
            }
          , "could not start vmem"
          );
      }

      master_agent_name = "agent-" + master.to_string() + "-0";

      master_agent_hostinfo = start_agent ( master
                                          , master_agent_name
                                          , "orchestrator"
                                          , orchestrator_hostinfo
                                          , gui_host
                                          , gui_port
                                          , log_host
                                          , log_port
                                          , gpi_socket
                                          , verbose
                                          , sdpa_home
                                          , log_dir
                                          , processes
                                          );

      return orchestrator_hostinfo;
    }

    namespace
    {
      template<typename It>
        void terminate_all_processes_of_a_kind
          (std::vector<It> const& entry_point_procs, component_type component)
      {
        std::string const kind
          ( component == component_type::worker ? "drts-kernel"
          : component == component_type::agent ? "agent"
          : component == component_type::orchestrator ? "orchestrator"
          : component == component_type::vmem ? "vmem"
          : throw std::logic_error ("invalid enum value")
          );

        std::vector<std::future<void>> terminates;

        for (It const& entry_point_processes : entry_point_procs)
        {
          using process_iter
            = typename decltype (entry_point_processes->second)::iterator;
          std::vector<pid_t> pids;
          std::vector<process_iter> to_erase;
          for ( process_iter it (entry_point_processes->second.begin())
              ; it != entry_point_processes->second.end()
              ; ++it
              )
          {
            if (fhg::util::starts_with (kind, it->first))
            {
              to_erase.emplace_back (it);
              pids.emplace_back (it->second);
            }
          }

          if (!pids.empty())
          {
            terminates.emplace_back
              ( std::async
                  ( std::launch::async
                  , [&kind, pids, to_erase, entry_point_processes]
                    {
                      std::cout << "terminating " << kind << " on "
                                << entry_point_processes->first.to_string()
                                << ": " << fhg::util::join (pids, " ") << "\n";

                      fhg::util::nest_exceptions<std::runtime_error>
                        ( [&entry_point_processes, &pids]
                          {
                            rif::client (entry_point_processes->first)
                              .kill (pids).get();
                          }
                        , ( boost::format ("Could not terminate %1% on %2%")
                          % kind
                          % entry_point_processes->first.to_string()
                          ).str()
                        );

                      for (process_iter const& iter : to_erase)
                      {
                        entry_point_processes->second.erase (iter);
                      }
                    }
                  )
              );
          }
        }

        fhg::util::wait_and_collect_exceptions (terminates);
      }
    }

    void processes_storage::shutdown
      ( component_type component
      , std::vector<fhg::rif::entry_point> const& rif_entry_points
      )
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

      terminate_all_processes_of_a_kind (iterators, component);

      for (decltype (_)::iterator const& it : iterators)
      {
        if (it->second.empty())
        {
          _.erase (it);
        }
      }
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
            terminate_all_processes_of_a_kind
              (iterators (_.begin(), _.end()), component);
          }
        );
    }
  }
}
