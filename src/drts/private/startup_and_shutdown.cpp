#include <drts/private/startup_and_shutdown.hpp>

#include <fhg/util/boost/serialization/path.hpp>
#include <fhg/util/boost/serialization/unordered_map.hpp>
#include <fhg/util/join.hpp>
#include <fhg/util/nest_exceptions.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/read_lines.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/starts_with.hpp>
#include <fhg/util/wait_and_collect_exceptions.hpp>

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

namespace
{
  void write_pidfile ( boost::filesystem::path const& processes_dir
                     , fhg::rif::entry_point const& entry_point
                     , std::string const& name
                     , pid_t pid
                     )
  {
    boost::filesystem::path const output_dir
      (processes_dir / entry_point.to_string());
    boost::filesystem::create_directories (output_dir);
    std::ofstream stream ((output_dir / (name + ".pid")).string());
    if (!stream || !(stream << pid))
    {
      throw std::runtime_error
        ("unable to write pidfile for " + name + " on " + entry_point.hostname);
    }
  }

  std::string build_parent_with_hostinfo
    (std::string const& name, fhg::drts::hostinfo_type const& hostinfo)
  {
    return ( boost::format ("%1%%%%2%%%%3%")
           % name
           % hostinfo.first
           % hostinfo.second
           ).str();
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

  struct segment_info_t
  {
    std::vector<fhg::rif::entry_point> entry_points;
    std::string master_name;
    fhg::drts::hostinfo_type master_hostinfo;
    segment_info_t ( std::vector<fhg::rif::entry_point> const& entry_points
                   , std::string const& master_name
                   )
      : entry_points (entry_points)
      , master_name (master_name)
    {}
  };

  fhg::drts::hostinfo_type start_agent
    ( fhg::rif::entry_point const& rif_entry_point
    , std::string const& name
    , std::string const& parent_name
    , fhg::drts::hostinfo_type const& parent_hostinfo
    , std::string const& gui_host
    , unsigned short gui_port
    , std::string const& log_host
    , unsigned short log_port
    , boost::optional<boost::filesystem::path> const& gpi_socket
    , bool verbose
    , boost::filesystem::path const& sdpa_home
    , boost::filesystem::path const& log_dir
    , boost::filesystem::path const& processes_dir
    )
  {
    std::cout << "I: starting agent: " << name << " on host "
              << rif_entry_point.hostname
              << " with parent " << parent_name << "\n";

    std::vector<std::string> agent_startup_arguments
      { "-u", "0"
      , "-n", name
      , "-m", build_parent_with_hostinfo (parent_name, parent_hostinfo)
      , "-a", gui_host + ":" + std::to_string (gui_port)
      };
    if (gpi_socket)
    {
      agent_startup_arguments.emplace_back ("--vmem-socket");
      agent_startup_arguments.emplace_back (gpi_socket->string());
    }

    std::pair<pid_t, std::vector<std::string>> const agent_startup_messages
      ( fhg::rif::client (rif_entry_point).execute_and_get_startup_messages
          ( "--startup-messages-pipe"
          , "OKAY"
          , sdpa_home / "bin" / "agent"
          , agent_startup_arguments
          , { {"FHGLOG_to_server", log_host + ":" + std::to_string (log_port)}
            , {"FHGLOG_level", verbose ? "TRACE" : "INFO"}
            , {"FHGLOG_to_file", (log_dir / (name + ".log")).string()}
            }
          ).get()
      );

    if (agent_startup_messages.second.size() != 2)
    {
      throw std::logic_error ( "could not start agent " + name
                             + ": expected 2 lines of startup messages"
                             );
    }

    write_pidfile ( processes_dir
                  , rif_entry_point
                  , name
                  , agent_startup_messages.first
                  );

    return { agent_startup_messages.second[0]
           , boost::lexical_cast<unsigned short> (agent_startup_messages.second[1])
           };
  }

  void add_plugin_option ( std::vector<std::string>& arguments
                         , std::string name
                         , const char* value
                         )
  {
    arguments.emplace_back ("-s");
    arguments.emplace_back ("plugin." + name + "=" + value);
  }
  void add_plugin_option ( std::vector<std::string>& arguments
                         , std::string name
                         , std::string value
                         )
  {
    add_plugin_option (arguments, name, value.c_str());
  }
  void add_plugin_option ( std::vector<std::string>& arguments
                         , std::string name
                         , unsigned long value
                         )
  {
    add_plugin_option (arguments, name, std::to_string (value).c_str());
  }

  std::vector<std::string> string_vector
    (std::vector<boost::filesystem::path> const& container)
  {
    std::vector<std::string> result;
    for (boost::filesystem::path const& elem : container)
    {
      result.emplace_back (elem.string());
    }
    return result;
  }

  void start_workers_for
    ( segment_info_t const& segment_info
    , fhg::drts::worker_description const& description
    , bool verbose
    , std::string const& gui_host
    , unsigned short gui_port
    , std::string const& log_host
    , unsigned short log_port
    , boost::filesystem::path const& processes_dir
    , boost::filesystem::path const& log_dir
    , boost::optional<boost::filesystem::path> const& gpi_socket
    , std::vector<boost::filesystem::path> const& app_path
    , boost::filesystem::path const& sdpa_home
    )
  {
     std::string name_prefix (fhg::util::join (description.capabilities, "+"));
     std::replace_if
       (name_prefix.begin(), name_prefix.end(), boost::is_any_of ("+#."), '_');

     std::cout << "I: starting " << name_prefix << " workers (segment "
               << segment_info.master_name << ", "
               << description.num_per_node << "/host, "
               << ( description.max_nodes == 0 ? "unlimited"
                  : description.max_nodes == 1 ? "unique"
                  : "global max: " + std::to_string (description.max_nodes)
                  )
               << ", " << description.shm_size << " SHM) with parent "
               << segment_info.master_name << " on host "
               << fhg::util::join
                    ( segment_info.entry_points
                    | boost::adaptors::transformed
                        ( [] (fhg::rif::entry_point const& entry_point)
                          {
                            return entry_point.hostname;
                          }
                        )
                    , ", "
                    )
               << "\n";

     std::vector<std::future<void>> startups;

     std::size_t num_nodes (0);
     for (fhg::rif::entry_point const& entry_point : segment_info.entry_points)
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
                   std::unordered_map<std::string, std::string> environment;

                   add_plugin_option
                     ( arguments
                     , "drts.master"
                     , build_parent_with_hostinfo
                         (segment_info.master_name, segment_info.master_hostinfo)
                     );
                   add_plugin_option (arguments, "drts.backlog", 1);
                   add_plugin_option
                     (arguments, "drts.max_reconnect_attempts", 128);
                   add_plugin_option ( arguments
                                     , "drts.gui_url"
                                     , gui_host + ":" + std::to_string (gui_port)
                                     );
                   add_plugin_option
                     ( arguments
                     , "drts.library_path"
                     , fhg::util::join (string_vector (app_path), ",")
                     );

                   std::vector<std::string> capabilities
                     (description.capabilities);

                   if (description.shm_size)
                   {
                     capabilities.emplace_back ("GPI");

                     arguments.emplace_back ("--virtual-memory-socket");
                     arguments.emplace_back (gpi_socket.get().string());
                     arguments.emplace_back ("--shared-memory-size");
                     arguments.emplace_back
                       (std::to_string (description.shm_size));
                   }

                   add_plugin_option ( arguments
                                     , "drts.capabilities"
                                     , fhg::util::join (capabilities, ",")
                                     );

                   if (description.socket)
                   {
                     add_plugin_option
                       (arguments, "drts.socket", description.socket.get());
                   }

                   environment.emplace ("FHGLOG_level", verbose ? "TRACE" : "INFO");

                   environment.emplace ( "LD_LIBRARY_PATH"
                                       , (sdpa_home / "lib").string() + ":"
                                       + (sdpa_home / "libexec" / "sdpa").string()
                                       );

                   environment.emplace ( "FHGLOG_to_server"
                                       , log_host + ":" + std::to_string (log_port)
                                       );

                   std::string const name
                     ( name_prefix + "-" + entry_point.hostname
                     + "-" + std::to_string (identity + 1)
                     + ( description.socket
                       ? ("." + std::to_string (description.socket.get()))
                       : std::string()
                       )
                     );

                   arguments.emplace_back ("-n");
                   arguments.emplace_back (name);

                   environment.emplace
                     ("FHGLOG_to_file", (log_dir / (name + ".log")).string());

                   std::pair<pid_t, std::vector<std::string>> const pid_and_startup_messages
                     ( fhg::rif::client (entry_point).execute_and_get_startup_messages
                       ( "--startup-messages-pipe"
                       , "OKAY"
                       , sdpa_home / "bin" / "drts-kernel"
                       , arguments
                       , environment
                       ).get()
                     );

                   if (!pid_and_startup_messages.second.empty())
                   {
                     throw std::runtime_error
                       ("could not start " + name + ": expected no startup messages");
                   }

                   write_pidfile ( processes_dir
                                 , entry_point
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
}

namespace fhg
{
  namespace drts
  {
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
      ( std::string gui_host
      , unsigned short gui_port
      , std::string log_host
      , unsigned short log_port
      , bool gpi_enabled
      , bool verbose
      , boost::optional<boost::filesystem::path> gpi_socket
      , std::vector<boost::filesystem::path> app_path
      , boost::filesystem::path sdpa_home
      , std::size_t number_of_groups
      , boost::filesystem::path state_dir
      , bool delete_logfiles
      , fhg::util::signal_handler_manager& signal_handler_manager
      , boost::optional<std::size_t> gpi_mem
      , boost::optional<std::chrono::seconds> vmem_startup_timeout
      , std::vector<worker_description> worker_descriptions
      , boost::optional<unsigned short> vmem_port
      , std::vector<fhg::rif::entry_point> const& rif_entry_points
      )
    {
      boost::filesystem::create_directories (state_dir);

      boost::filesystem::path const nodefile (state_dir / "nodefile");
      {
        std::ofstream nodefile_stream (nodefile.string());

        if (!nodefile_stream)
        {
          throw std::runtime_error
            ( ( boost::format ("Could not create nodefile %1%: %2%")
              % nodefile
              % strerror (errno)
              )
            . str()
            );
        }

        for (fhg::rif::entry_point const& entry_point : rif_entry_points)
        {
          nodefile_stream << entry_point.hostname << "\n";
        }

        if (!nodefile_stream)
        {
          throw std::runtime_error
            ( ( boost::format ("Could not write to nodefile %1%: %2%")
              % nodefile
              % strerror (errno)
              )
            . str()
            );
        }
      }

      fhg::rif::entry_point const& master (rif_entry_points.front());

      if (number_of_groups > rif_entry_points.size())
      {
        throw std::invalid_argument
          ( "number of groups must not be larger than number of rif entry points: "
          + std::to_string (number_of_groups) + " > "
          + std::to_string (rif_entry_points.size())
          );
      }

      boost::filesystem::path const log_dir (state_dir / "log");
      boost::filesystem::path const processes_dir (state_dir / "processes");

      boost::filesystem::create_directories (processes_dir);

      if (!boost::filesystem::is_empty (processes_dir))
      {
        std::cout << "W: there are still pid files in " << processes_dir
                  << ", please make sure that the previous drts is stopped.\n"
                  << "I: to stop the old one: run "
                  << (sdpa_home / "bin" / "sdpa")
                  << " stop -s " << state_dir << "\n";
        throw std::runtime_error ("state directory is not clean");
      }

      if (delete_logfiles)
      {
        boost::filesystem::remove_all (log_dir);
      }
      boost::filesystem::create_directories (log_dir);

      struct stop_drts_on_failure
      {
        ~stop_drts_on_failure()
        {
          if (!_successful)
          {
            shutdown (_state_dir, _rif_entry_points);
          }
        }
        void startup_successful()
        {
          _successful = true;
        }
        boost::filesystem::path const& _state_dir;
        std::vector<fhg::rif::entry_point> const& _rif_entry_points;
        bool _successful;
      } stop_drts_on_failure = {state_dir, rif_entry_points, false};

      fhg::util::scoped_signal_handler interrupt_signal_handler
        ( signal_handler_manager
        ,  SIGINT
        , [] (int, siginfo_t*, void*)
          {
            throw std::runtime_error ("Canceled...");
          }
        );

      std::cout << "I: using nodefile: " << nodefile << "\n"
                << "starting base sdpa components on " << master.hostname << "...\n"
                << "I: sending log events to: "
                << log_host << ":" << log_port << "\n"
                << "I: sending execution events to: "
                << gui_host << ":" << gui_port << "\n";

      std::pair<pid_t, std::vector<std::string>> const orchestrator_startup_messages
        ( fhg::util::nest_exceptions<std::runtime_error>
            ( [&]
              {
                return rif::client (master).execute_and_get_startup_messages
                  ( "--startup-messages-pipe"
                  , "OKAY"
                  , sdpa_home / "bin" / "orchestrator"
                  , {"-u", "0", "-n", "orchestrator"}
                  , { {"FHGLOG_to_server", log_host + ":" + std::to_string (log_port)}
                    , {"FHGLOG_level", verbose ? "TRACE" : "INFO"}
                    , {"FHGLOG_to_file", (state_dir / "log" / "orchestrator.log").string()}
                    }
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

      write_pidfile ( processes_dir
                    , master
                    , "orchestrator"
                    , orchestrator_startup_messages.first
                    );

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
                        std::pair<pid_t, std::vector<std::string>> const
                          startup_messages
                          ( rif::client (entry_point).execute_and_get_startup_messages
                              ( "--startup-messages-pipe"
                              , "OKAY"
                              , sdpa_home / "bin" / "gpi-space"
                              , { "--log-host", log_host
                                , "--log-port", std::to_string (log_port)
                                , "--log-level", verbose ? "TRACE" : "INFO"
                                , "--gpi-mem", std::to_string (gpi_mem.get())
                                , "--socket", gpi_socket.get().string()
                                , "--port", std::to_string (vmem_port.get())
                                , "--gpi-api"
                                , rif_entry_points.size() > 1 ? "gaspi" : "fake"
                                , "--gpi-timeout", std::to_string (vmem_startup_timeout.get().count())
                                }
                              , { {"GASPI_MFILE", nodefile.string()}
                                , {"GASPI_MASTER", master.hostname}
                                , {"GASPI_SOCKET", "0"}
                                , { "GASPI_TYPE"
                                  , entry_point == master
                                  ? "GASPI_MASTER"
                                  : "GASPI_WORKER"
                                  }
                                , {"GASPI_SET_NUMA_SOCKET", "0"}
                                }
                              ).get()
                          );

                        if (!startup_messages.second.empty())
                        {
                          throw std::logic_error
                            ( entry_point.hostname
                            + ": expected no startup messages"
                            );
                        }

                        write_pidfile ( processes_dir
                                      , entry_point
                                      , "vmem"
                                      , startup_messages.first
                                      );
                      }
                    )
                );
              }

              fhg::util::wait_and_collect_exceptions (futures);
            }
          , "could not start vmem"
          );
      }

      std::vector<segment_info_t> segment_info;
      {
        if (number_of_groups == 1)
        {
          segment_info.emplace_back
            (rif_entry_points, "agent-" + master.hostname + "-0");
        }
        else
        {
          std::size_t const hosts_per_group
            (rif_entry_points.size() / number_of_groups);
          std::size_t const remaining_hosts
            (rif_entry_points.size() % number_of_groups);

          for (std::size_t i (0); i < number_of_groups; i += hosts_per_group)
          {
            segment_info.emplace_back
              ( std::vector<fhg::rif::entry_point>
                  ( rif_entry_points.begin() + i
                  , rif_entry_points.begin() + i + hosts_per_group
                  )
              , "agent-" + rif_entry_points[i].hostname + "-1"
              );
          }

          segment_info.back().entry_points.insert
            ( segment_info.back().entry_points.end()
            , segment_info.back().entry_points.rbegin() + remaining_hosts
            , segment_info.back().entry_points.rbegin()
            );
        }
      }

      fhg::util::nest_exceptions<std::runtime_error>
        ( [&]
          {
            std::string const master_agent_name
              ("agent-" + master.hostname + "-0");
            hostinfo_type const master_agent_hostinfo
              ( start_agent ( master
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
                            , processes_dir
                            )
              );

            if (segment_info.size() == 1)
            {
              segment_info.front().master_hostinfo = master_agent_hostinfo;
            }
            else
            {
              std::vector<std::future<void>> startups;
              for (segment_info_t& info : segment_info)
              {
                startups.emplace_back ( std::async
                                          ( std::launch::async
                                          , [&]
                                            {
                                              info.master_hostinfo = start_agent
                                                ( info.entry_points.front()
                                                , info.master_name
                                                , master_agent_name
                                                , master_agent_hostinfo
                                                , gui_host
                                                , gui_port
                                                , log_host
                                                , log_port
                                                , gpi_socket
                                                , verbose
                                                , sdpa_home
                                                , log_dir
                                                , processes_dir
                                                );
                                            }
                                          )
                                      );
              }
              fhg::util::wait_and_collect_exceptions (startups);
            }
          }
          , "at least one agent could not be started!"
          );

      fhg::util::nest_exceptions<std::runtime_error>
        ( [&]
          {
            for (segment_info_t const& info : segment_info)
            {
              for (worker_description const& description : worker_descriptions)
              {
                start_workers_for ( info
                                  , description
                                  , verbose
                                  , gui_host
                                  , gui_port
                                  , log_host
                                  , log_port
                                  , processes_dir
                                  , log_dir
                                  , gpi_socket
                                  , app_path
                                  , sdpa_home
                                  );
              }
            }
          }
          , "at least one worker could not be started!"
          );

      stop_drts_on_failure.startup_successful();

      return orchestrator_hostinfo;
    }

    namespace
    {
      boost::iterator_range<boost::filesystem::directory_iterator>
        directory_range (boost::filesystem::path const& path)
      {
        return { boost::filesystem::directory_iterator (path)
               , boost::filesystem::directory_iterator()
               };
      }


      void terminate_processes_on_host ( std::string const& name
                                       , fhg::rif::entry_point const& entry_point
                                       , std::vector<pid_t> const& pids
                                       )
      {
        if (!pids.empty())
        {
          std::cout << "terminating " << name << " on " << entry_point.hostname
                    << ": " << fhg::util::join (pids, " ") << "\n";
          try
          {
            rif::client (entry_point).kill (pids).get();
          }
          catch (...)
          {
            std::throw_with_nested
              ( std::runtime_error
                  (( boost::format ("Could not terminate %1% on %2%")
                   % name
                   % entry_point.hostname
                   ).str()
                  )
              );
          }
        }
      }

      void terminate_all_processes_of_a_kind
        ( boost::filesystem::path const& state_dir
        , std::string const& kind
        , std::vector<fhg::rif::entry_point> const& rif_entry_points
        )
      {
        boost::filesystem::path const processes_dir (state_dir / "processes");

        std::vector<std::future<void>> terminates;

        for (fhg::rif::entry_point const& entry_point : rif_entry_points)
        {
          std::set<boost::filesystem::path> pidfiles;

          for ( boost::filesystem::directory_entry const& entry
              : directory_range (processes_dir / entry_point.to_string())
              | boost::adaptors::filtered
                  ( [&kind] (boost::filesystem::directory_entry const& entry)
                    {
                      return entry.path().extension() == ".pid"
                        && entry.status().type() == boost::filesystem::regular_file
                        && fhg::util::starts_with
                             (kind, entry.path().stem().string())
                        ;
                    }
                  )
              )
          {
            pidfiles.emplace (entry.path());
          }

          terminates.emplace_back
            ( std::async
                ( std::launch::async
                , [kind, entry_point, pidfiles]
                  {
                    std::vector<int> pids;
                    for (boost::filesystem::path const& pidfile : pidfiles)
                    {
                      pids.emplace_back ( boost::lexical_cast<pid_t>
                                            (fhg::util::read_file (pidfile))
                                        );
                    }
                    terminate_processes_on_host (kind, entry_point, pids);
                    for (boost::filesystem::path const& pidfile : pidfiles)
                    {
                      boost::filesystem::remove (pidfile);
                    }
                  }
                )
            );
        }

        fhg::util::wait_and_collect_exceptions (terminates);
      }
    }

    //! \todo learn enum class
    namespace components_type
    {
      enum components_type
      {
        vmem = 1 << 1,
        orchestrator = 1 << 2,
        agent = 1 << 3,
        worker = 1 << 4,
      };
    }

    void shutdown ( boost::filesystem::path const& state_dir
                  , boost::optional<components_type::components_type> components
                  , std::vector<fhg::rif::entry_point> const& rif_entry_points
                  )
    {
      if (!components)
      {
        components = components_type::components_type
          ( components_type::vmem | components_type::orchestrator
          | components_type::agent | components_type::worker
          );
      }

      if (components.get() & components_type::worker)
      {
        terminate_all_processes_of_a_kind
          (state_dir, "drts-kernel", rif_entry_points);
      }
      if (components.get() & components_type::agent)
      {
        terminate_all_processes_of_a_kind (state_dir, "agent", rif_entry_points);
      }
      if (components.get() & components_type::vmem)
      {
        terminate_all_processes_of_a_kind (state_dir, "vmem", rif_entry_points);
      }
      if (components.get() & components_type::orchestrator)
      {
        terminate_all_processes_of_a_kind
          (state_dir, "orchestrator", rif_entry_points);
      }

        for ( boost::filesystem::directory_entry const& entry
            : directory_range (state_dir / "processes")
            | boost::adaptors::filtered
                ( [] (boost::filesystem::directory_entry const& entry)
                  {
                    return boost::filesystem::is_empty (entry.path());
                  }
                )
            )
        {
          boost::filesystem::remove (entry.path());
        }

    }

    void shutdown ( boost::filesystem::path const& state_dir
                  , std::vector<fhg::rif::entry_point> const& rif_entry_points
                  )
    {
      shutdown (state_dir, boost::none, rif_entry_points);

      boost::filesystem::remove (state_dir / "processes");
      boost::filesystem::remove (state_dir / "nodefile");
    }
  }
}
