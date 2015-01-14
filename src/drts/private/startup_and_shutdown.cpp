#include <drts/private/startup_and_shutdown.hpp>

#include <fhg/util/boost/serialization/path.hpp>
#include <fhg/util/boost/serialization/unordered_map.hpp>
#include <fhg/util/join.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/read_lines.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/starts_with.hpp>

#include <rpc/client.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/regex.hpp>
#include <boost/thread/scoped_thread.hpp>

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
  template<typename R, typename... Args>
    R rexec ( std::string const& host, unsigned short port
            , std::string const& function_name
            , Args... args
            )
  {
    boost::asio::io_service io_service;
    boost::asio::io_service::work const io_service_work_ (io_service);
    boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable> const
      io_service_thread ([&io_service] { io_service.run(); });

    fhg::rpc::remote_endpoint endpoint
      ( io_service
      , host, port
      , fhg::rpc::exception::serialization_functions()
      );

    struct stop_io_service_on_scope_exit
    {
      ~stop_io_service_on_scope_exit()
      {
        _io_service.stop();
      }
      boost::asio::io_service& _io_service;
    } foo {io_service};

    return fhg::rpc::sync_remote_function<R (Args...)>
      (endpoint, function_name) (args...);
  }

  std::pair<pid_t, std::vector<std::string>> remote_execute_and_get_startup_messages
    ( std::string const& host, unsigned short port
    , std::string startup_messages_pipe_option
    , std::string end_sentinel_value
    , boost::filesystem::path command
    , std::vector<std::string> arguments
    , std::unordered_map<std::string, std::string> environment
    )
  {
    return rexec<std::pair<pid_t, std::vector<std::string>>>
      ( host, port
      , "execute_and_get_startup_messages"
      , startup_messages_pipe_option
      , end_sentinel_value
      , command
      , arguments
      , environment
      );
  }

  void write_pidfile ( boost::filesystem::path const& processes_dir
                     , std::string const& host
                     , std::string const& name
                     , pid_t pid
                     )
  {
    boost::filesystem::create_directories (processes_dir / host);
    std::ofstream ((processes_dir / host / (name + ".pid")).string()) << pid;
  }

  using hostinfo_type = std::pair<std::string, unsigned short>;

  void write_hostinfo ( boost::filesystem::path const& state_dir
                      , std::string const& name
                      , hostinfo_type const& hostinfo
                      )
  {
    std::ofstream ((state_dir / (name + ".host")).string()) << hostinfo.first;
    std::ofstream ((state_dir / (name + ".port")).string()) << hostinfo.second;
  }

  std::string build_parent_with_hostinfo
    (std::string const& name, hostinfo_type const& hostinfo)
  {
    return ( boost::format ("%1%%%%2%%%%3%")
           % name
           % hostinfo.first
           % hostinfo.second
           ).str();
  }

  template<typename Exception, typename... ExceptionArgs, typename Fun>
    auto nest_exceptions (Fun&& fun, ExceptionArgs... exception_args)
      -> decltype (fun())
  {
    try
    {
      return fun();
    }
    catch (...)
    {
      std::throw_with_nested
        (Exception (std::forward<ExceptionArgs> (exception_args)...));
    }
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
    std::vector<std::string> hosts;
    std::string master_name;
    hostinfo_type master_hostinfo;
    segment_info_t (std::vector<std::string> hosts, std::string master_name)
      : hosts (hosts)
      , master_name (master_name)
    {}
  };

  hostinfo_type start_agent
    ( std::string const& host
    , std::string const& name
    , std::string const& parent_name
    , hostinfo_type const& parent_hostinfo
    , std::string const& gui_host
    , unsigned short gui_port
    , std::string const& log_host
    , unsigned short log_port
    , boost::optional<boost::filesystem::path> const& gpi_socket
    , bool verbose
    , boost::filesystem::path const& sdpa_home
    , boost::filesystem::path const& log_dir
    , boost::filesystem::path const& processes_dir
    , unsigned short rif_port
    )
  {
    std::cout << "I: starting agent: " << name << " on host " << host
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
      ( remote_execute_and_get_startup_messages
          ( host, rif_port
          , "--startup-messages-pipe"
          , "OKAY"
          , sdpa_home / "bin" / "agent"
          , agent_startup_arguments
          , { {"FHGLOG_to_server", log_host + ":" + std::to_string (log_port)}
            , {"FHGLOG_level", verbose ? "TRACE" : "INFO"}
            , {"FHGLOG_to_file", (log_dir / (name + ".log")).string()}
            }
          )
      );

    if (agent_startup_messages.second.size() != 2)
    {
      throw std::logic_error ( "could not start agent " + name
                             + ": expected 2 lines of startup messages"
                             );
    }

    write_pidfile (processes_dir, host, name, agent_startup_messages.first);

    return { agent_startup_messages.second[0]
           , boost::lexical_cast<unsigned short> (agent_startup_messages.second[1])
           };
  }

  void wait_and_collect_exceptions (std::vector<std::future<void>>& futures)
  {
    std::vector<std::string> accumulated_whats;
    for (std::future<void>& future : futures)
    {
      try
      {
        future.get();
      }
      catch (std::exception const& ex)
      {
        accumulated_whats.emplace_back (ex.what());
      }
    }
    if (!accumulated_whats.empty())
    {
      throw std::runtime_error (fhg::util::join (accumulated_whats, ", "));
    }
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
    , unsigned short rif_port
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
               << fhg::util::join (segment_info.hosts, ", ") << "\n";

     std::vector<std::future<void>> startups;

     std::size_t num_nodes (0);
     for (std::string const& host : segment_info.hosts)
     {
       for ( unsigned long identity (0)
           ; identity < description.num_per_node
           ; ++identity
           )
       {
         startups.emplace_back
           ( std::async
               ( std::launch::async
               , [&, host, identity]
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
                     ( name_prefix + "-" + host
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
                     ( remote_execute_and_get_startup_messages
                       ( host, rif_port
                       , "--startup-messages-pipe"
                       , "OKAY"
                       , sdpa_home / "bin" / "drts-kernel"
                       , arguments
                       , environment
                       )
                     );

                   if (!pid_and_startup_messages.second.empty())
                   {
                     throw std::runtime_error
                       ("could not start " + name + ": expected no startup messages");
                   }

                   write_pidfile ( processes_dir
                                 , host
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

     wait_and_collect_exceptions (startups);
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

    void startup ( std::string gui_host
                 , unsigned short gui_port
                 , std::string log_host
                 , unsigned short log_port
                 , bool gpi_enabled
                 , bool verbose
                 , boost::optional<boost::filesystem::path> gpi_socket
                 , std::vector<boost::filesystem::path> app_path
                 , boost::filesystem::path sdpa_home
                 , std::size_t number_of_groups
                 , boost::filesystem::path nodefile
                 , boost::filesystem::path state_dir
                 , bool delete_logfiles
                 , fhg::util::signal_handler_manager& signal_handler_manager
                 , boost::optional<std::size_t> gpi_mem
                 , boost::optional<std::chrono::seconds> vmem_startup_timeout
                 , std::vector<worker_description> worker_descriptions
                 , boost::optional<unsigned short> vmem_port
                 , unsigned short rif_port
                 )
    {
      std::vector<std::string> nodefile_content
        (fhg::util::read_lines (nodefile));
      std::vector<std::string> const hosts
        ( nodefile_content.begin()
        , std::unique (nodefile_content.begin(), nodefile_content.end())
        );

      if (hosts.empty())
      {
        throw std::invalid_argument ("hostfile empty");
      }

      boost::filesystem::create_directories (state_dir);

      boost::filesystem::path const uniqued_nodefile (state_dir / "nodefile");
      {
        std::ofstream uniqued_nodefile_stream (uniqued_nodefile.string());

        if (!uniqued_nodefile_stream)
        {
          throw std::runtime_error
            ( ( boost::format ("Could not create nodefile %1%: %2%")
              % uniqued_nodefile
              % strerror (errno)
              )
            . str()
            );
        }

        for (std::string const& host : hosts)
        {
          uniqued_nodefile_stream << host << "\n";
        }

        if (!uniqued_nodefile_stream)
        {
          throw std::runtime_error
            ( ( boost::format ("Could not write to nodefile %1%: %2%")
              % uniqued_nodefile
              % strerror (errno)
              )
            . str()
            );
        }
      }

      std::string const master (hosts.front());

      if (number_of_groups > hosts.size())
      {
        throw std::invalid_argument
          ( "number of groups must not be larger than number of hosts: "
          + std::to_string (number_of_groups) + " > "
          + std::to_string (hosts.size())
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
        stop_drts_on_failure
            (boost::filesystem::path const& state_dir, unsigned short rif_port)
          : _state_dir (state_dir)
          , _rif_port (rif_port)
        {}
        ~stop_drts_on_failure()
        {
          if (!_successful)
          {
            shutdown (_state_dir, _rif_port);
          }
        }
        void startup_successful()
        {
          _successful = true;
        }
        boost::filesystem::path const& _state_dir;
        unsigned short _rif_port;
        bool _successful {false};
      } stop_drts_on_failure = {state_dir, rif_port};

      fhg::util::scoped_signal_handler interrupt_signal_handler
        ( signal_handler_manager
        ,  SIGINT
        , [] (int, siginfo_t*, void*)
          {
            throw std::runtime_error ("Canceled...");
          }
        );

      std::cout << "I: using nodefile: " << nodefile << "\n"
                << "starting base sdpa components on " << master << "...\n"
                << "I: sending log events to: "
                << log_host << ":" << log_port << "\n"
                << "I: sending execution events to: "
                << gui_host << ":" << gui_port << "\n";

      std::pair<pid_t, std::vector<std::string>> const orchestrator_startup_messages
        ( nest_exceptions<std::runtime_error>
            ( [&]
              {
                return remote_execute_and_get_startup_messages
                  ( master, rif_port
                  , "--startup-messages-pipe"
                  , "OKAY"
                  , sdpa_home / "bin" / "orchestrator"
                  , {"-u", "0", "-n", "orchestrator"}
                  , { {"FHGLOG_to_server", log_host + ":" + std::to_string (log_port)}
                    , {"FHGLOG_level", verbose ? "TRACE" : "INFO"}
                    , {"FHGLOG_to_file", (state_dir / "log" / "orchestrator.log").string()}
                    }
                  );
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
      write_hostinfo (state_dir, "orchestrator", orchestrator_hostinfo);
      write_hostinfo (state_dir, "orchestrator.rpc", orchestrator_rpc_hostinfo);

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

        nest_exceptions<std::runtime_error>
          ( [&]
            {
              std::vector<std::future<void>> futures;

              for (std::string const& host : hosts)
              {
                futures.emplace_back
                  ( std::async
                      ( std::launch::async
                      , [&, host]
                      {
                        std::pair<pid_t, std::vector<std::string>> const
                          startup_messages
                          ( remote_execute_and_get_startup_messages
                              ( host, rif_port
                              , "--startup-messages-pipe"
                              , "OKAY"
                              , sdpa_home / "bin" / "gpi-space"
                              , { "--log-host", log_host
                                , "--log-port", std::to_string (log_port)
                                , "--log-level", verbose ? "TRACE" : "INFO"
                                , "--gpi-mem", std::to_string (gpi_mem.get())
                                , "--socket", gpi_socket.get().string()
                                , "--port", std::to_string (vmem_port.get())
                                , "--gpi-api", hosts.size() > 1 ? "gaspi" : "fake"
                                , "--gpi-timeout", std::to_string (vmem_startup_timeout.get().count())
                                }
                              , { {"GASPI_MFILE", uniqued_nodefile.string()}
                                , {"GASPI_MASTER", master}
                                , {"GASPI_SOCKET", "0"}
                                , { "GASPI_TYPE"
                                  , host == master ? "GASPI_MASTER" : "GASPI_WORKER"
                                  }
                                , {"GASPI_SET_NUMA_SOCKET", "0"}
                                }
                              )
                          );

                        if (!startup_messages.second.empty())
                        {
                          throw std::logic_error
                            (host + ": expected no startup messages");
                        }

                        write_pidfile
                          (processes_dir, host, "vmem", startup_messages.first);
                      }
                    )
                );
              }

              wait_and_collect_exceptions (futures);
            }
          , "could not start vmem"
          );
      }

      std::vector<segment_info_t> segment_info;
      {
        if (number_of_groups == 1)
        {
          segment_info.emplace_back (hosts, "agent-" + master + "-0");
        }
        else
        {
          std::size_t const hosts_per_group (hosts.size() / number_of_groups);
          std::size_t const remaining_hosts (hosts.size() % number_of_groups);

          for (std::size_t i (0); i < number_of_groups; i += hosts_per_group)
          {
            segment_info.emplace_back
              ( std::vector<std::string>
                  (hosts.begin() + i, hosts.begin() + i + hosts_per_group)
              , "agent-" + hosts[i] + "-1"
              );
          }

          segment_info.back().hosts.insert
            ( segment_info.back().hosts.end()
            , segment_info.back().hosts.rbegin() + remaining_hosts
            , segment_info.back().hosts.rbegin()
            );
        }
      }

      nest_exceptions<std::runtime_error>
        ( [&]
          {
            std::string const master_agent_name ("agent-" + master + "-0");
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
                            , rif_port
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
                                                ( info.hosts.front()
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
                                                , rif_port
                                                );
                                            }
                                          )
                                      );
              }
              wait_and_collect_exceptions (startups);
            }
          }
          , "at least one agent could not be started!"
          );

      nest_exceptions<std::runtime_error>
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
                                  , rif_port
                                  );
              }
            }
          }
          , "at least one worker could not be started!"
          );

      stop_drts_on_failure.startup_successful();
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
                                       , std::string const& host
                                       , unsigned short rif_port
                                       , std::vector<pid_t> const& pids
                                       )
      {
        if (!pids.empty())
        {
          std::cout << "terminating " << name << " on " << host
                    << ": " << fhg::util::join (pids, " ") << "\n";
          try
          {
            rexec<void> (host, rif_port, "kill", pids);
          }
          catch (...)
          {
            std::throw_with_nested
              ( std::runtime_error
                  (( boost::format ("Could not terminate %1% on %2%")
                   % name
                   % host
                   ).str()
                  )
              );
          }
        }
      }

      void terminate_all_processes_of_a_kind
        ( boost::filesystem::path const& state_dir
        , std::string const& kind
        , std::vector<std::string> hosts
        , unsigned short rif_port
        )
      {
        boost::filesystem::path const processes_dir (state_dir / "processes");

        if (hosts.empty())
        {
          for ( boost::filesystem::directory_entry const& entry
              : directory_range (processes_dir)
              | boost::adaptors::filtered
                  ( [] (boost::filesystem::directory_entry const& entry)
                    {
                      return boost::filesystem::is_directory (entry.path());
                    }
                  )
              )
          {
            hosts.emplace_back (entry.path().filename().string());
          }
        }

        std::vector<std::future<void>> terminates;

        for (std::string const& host : hosts)
        {
          std::set<boost::filesystem::path> pidfiles;

          for ( boost::filesystem::directory_entry const& entry
              : directory_range (processes_dir / host)
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
                , [kind, host, rif_port, pidfiles]()
                  {
                    std::vector<int> pids;
                    for (boost::filesystem::path const& pidfile : pidfiles)
                    {
                      pids.emplace_back ( boost::lexical_cast<pid_t>
                                            (fhg::util::read_file (pidfile))
                                        );
                    }
                    terminate_processes_on_host (kind, host, rif_port, pids);
                    for (boost::filesystem::path const& pidfile : pidfiles)
                    {
                      boost::filesystem::remove (pidfile);
                    }
                  }
                )
            );
        }

        wait_and_collect_exceptions (terminates);

        for ( boost::filesystem::directory_entry const& entry
            : directory_range (processes_dir)
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
                  , std::vector<std::string> const& hosts
                  , unsigned short rif_port
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
          (state_dir, "drts-kernel", hosts, rif_port);
      }
      if (components.get() & components_type::agent)
      {
        terminate_all_processes_of_a_kind
          (state_dir, "agent", hosts, rif_port);
      }
      if (components.get() & components_type::vmem)
      {
        terminate_all_processes_of_a_kind (state_dir, "vmem", hosts, rif_port);
      }
      if (components.get() & components_type::orchestrator)
      {
        boost::filesystem::remove (state_dir / "orchestrator.host");
        boost::filesystem::remove (state_dir / "orchestrator.port");
        boost::filesystem::remove (state_dir / "orchestrator.rpc.host");
        boost::filesystem::remove (state_dir / "orchestrator.rpc.port");

        terminate_all_processes_of_a_kind
          (state_dir, "orchestrator", hosts, rif_port);
      }
    }

    void shutdown
      (boost::filesystem::path const& state_dir, unsigned short rif_port)
    {
      shutdown (state_dir, boost::none, {}, rif_port);

      boost::filesystem::remove (state_dir / "processes");
      boost::filesystem::remove (state_dir / "nodefile");
    }
  }
}
