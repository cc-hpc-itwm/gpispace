#include <drts/private/startup_and_shutdown.hpp>

#include <fhg/syscall.hpp>
#include <fhg/util/getenv.hpp>
#include <fhg/util/join.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/read_lines.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/starts_with.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <rif/execute_and_get_startup_messages.hpp>

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
  //! \note Works around sshd's MaxStartups:
  // Specifies the maximum number of concurrent unauthenticated con-
  // nections to the sshd daemon.  Additional connections will be
  // dropped until authentication succeeds or the LoginGraceTime
  // expires for a connection.	The default is 10.
  //  (see man sshd_config)
  // We limit to 3 to "allow" parallel testing.
  constexpr const std::size_t parallel_ssh_limit (3);
  std::mutex parallel_ssh_limit_mutex;
  std::condition_variable parallel_ssh_limit_freed_condition;
  std::unordered_map<std::string, std::size_t> parallel_ssh_limit_counter;
  struct parallel_ssh_limiter
  {
    parallel_ssh_limiter (std::string const& host)
      : _host (host)
    {
      std::unique_lock<std::mutex> lock (parallel_ssh_limit_mutex);
      parallel_ssh_limit_freed_condition.wait
        ( lock
        , [this] { return parallel_ssh_limit_counter[_host] < parallel_ssh_limit; }
        );
      ++parallel_ssh_limit_counter[_host];
    }
    ~parallel_ssh_limiter()
    {
      {
        std::unique_lock<std::mutex> const _ (parallel_ssh_limit_mutex);
        if (--parallel_ssh_limit_counter[_host] == 0)
        {
          parallel_ssh_limit_counter.erase (_host);
        }
      }
      parallel_ssh_limit_freed_condition.notify_one();
    }
    std::string const& _host;
  };

  void system (std::string const& command, std::string const& description)
  {
    if (int ec = fhg::util::system_with_blocked_SIGCHLD (command.c_str()))
    {
      throw std::runtime_error
        (( boost::format ("Could not '%3%': error code '%1%', command was '%2%'")
         % ec
         % command
         % description
         ).str()
        );
    }
  }

  std::string const ssh_opts
    ("-q -x -T -n -C -4 -o CheckHostIP=no -o StrictHostKeyChecking=no");

  void rexec (std::string const& host, std::string const& command)
  {
    parallel_ssh_limiter const _ (host);

    system ( "ssh " + ssh_opts + " " + host
           + " /usr/bin/env LD_LIBRARY_PATH='"
           + fhg::util::getenv ("LD_LIBRARY_PATH").get_value_or ("")
           + "' '" + command + "'"
           , "rexec"
           );
  }

  std::pair<pid_t, std::vector<std::string>> remote_execute_and_get_startup_messages
    ( std::string const& host
    , std::string const& startup_messages_pipe_option
    , std::string const& end_sentinel_value
    , boost::filesystem::path const& command
    , std::vector<std::string> const& arguments
    , std::unordered_map<std::string, std::string> const& environment
    , boost::filesystem::path const& sdpa_home
    )
  {
    std::string environment_string;
    for (std::pair<std::string, std::string> const& var : environment)
    {
      environment_string
        += " --environment " + var.first + "='" + var.second + "'";
    }
    std::string arguments_string;
    for (std::string const& arg : arguments)
    {
      arguments_string += " --arguments '" + arg + "'";
    }

    std::vector<std::string> lines;

    {
      parallel_ssh_limiter const _ (host);

      struct scoped_popen
      {
        scoped_popen (const char* command, const char* type)
          : _ (fhg::syscall::popen (command, type))
          , _command (command)
        {}
        ~scoped_popen()
        {
          int const status (fhg::syscall::pclose (_));

          if (WIFEXITED (status) && WEXITSTATUS (status) != 0)
          {
            throw std::runtime_error
              ("'" + _command + "' failed: " + std::to_string (WEXITSTATUS (status)));
          }
          else if (WIFSIGNALED (status))
          {
            throw std::runtime_error
              ("'" + _command + "' signaled: " + std::to_string (WTERMSIG (status)));
          }
        }
        FILE* _;
        std::string _command;
      } process
          ( ( "ssh " + ssh_opts + " " + host
            + " /usr/bin/env LD_LIBRARY_PATH='"
            + fhg::util::getenv ("LD_LIBRARY_PATH").get_value_or ("")
            + "' '" + (sdpa_home / "bin" / "start-and-fork").string() + "'"
            + " --end-sentinel-value " + end_sentinel_value
            + " --startup-messages-pipe-option " + startup_messages_pipe_option
            + " --command '" + command.string() + "'"
            + arguments_string
            + environment_string
            ).c_str()
          , "r"
          );

      struct free_on_scope_exit
      {
        ~free_on_scope_exit()
        {
          free (_);
          _ = nullptr;
        }
        char* _ {nullptr};
      } line;
      std::size_t length (0);

      ssize_t read;
      while ((read = getline (&line._, &length, process._)) != -1)
      {
        lines.emplace_back (line._, read - 1);
      }
    }

    return { boost::lexical_cast<pid_t> (lines.at (0))
           , {lines.begin() + 1, lines.end()}
           };
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
          ( host
          , "--startup-messages-pipe"
          , "OKAY"
          , sdpa_home / "bin" / "agent"
          , agent_startup_arguments
          , { {"FHGLOG_to_server", log_host + ":" + std::to_string (log_port)}
            , {"FHGLOG_level", verbose ? "TRACE" : "INFO"}
            , {"FHGLOG_to_file", (log_dir / (name + ".log")).string()}
            }
          , sdpa_home
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

  void start_workers_for
    ( segment_info_t const& segment_info
    , fhg::drts::worker_description const& description
    , bool verbose
    , std::string const& gui_host
    , unsigned short gui_port
    , std::string const& log_host
    , unsigned short log_port
    , boost::filesystem::path const& state_dir
    , boost::optional<boost::filesystem::path> const& gpi_socket
    , std::vector<boost::filesystem::path> const& app_path
    , boost::filesystem::path const& sdpa_home
    )
  {
     std::string name (fhg::util::join (description.capabilities, "+"));
     std::replace_if (name.begin(), name.end(), boost::is_any_of ("+#."), '_');

     boost::filesystem::path const script_start_drts
       (sdpa_home / "libexec" / "sdpa" / "scripts" / "start-drts");

     std::cout << "I: starting " << name << " workers (segment "
               << segment_info.master_name << ", "
               << description.num_per_node << "/host, "
               << ( description.max_nodes == 0 ? "unlimited"
                  : description.max_nodes == 1 ? "unique"
                  : "global max: " + std::to_string (description.max_nodes)
                  )
               << ", " << description.shm_size << " SHM)...\n";

     std::string const parent
       ( build_parent_with_hostinfo
           (segment_info.master_name, segment_info.master_hostinfo)
       );

     std::vector<std::future<void>> startups;
     std::size_t num_nodes (0);
     for (std::string const& host : segment_info.hosts)
     {
       //! \todo Quoting?
       std::vector<std::string> argv;
       if (verbose)
       {
         argv.emplace_back ("-v");
       }
       argv.emplace_back ("-c");
       argv.emplace_back (std::to_string (description.num_per_node));
       argv.emplace_back ("-n");
       argv.emplace_back (name);
       argv.emplace_back ("-m");
       argv.emplace_back (parent);
       argv.emplace_back ("-g");
       argv.emplace_back (gui_host + ":" + std::to_string (gui_port));
       argv.emplace_back ("-l");
       argv.emplace_back (log_host + ":" + std::to_string (log_port));
       for (std::string const& capability : description.capabilities)
       {
         argv.emplace_back ("-C");
         argv.emplace_back (capability);
       }
       argv.emplace_back ("-s");
       argv.emplace_back (state_dir.string());
       if (gpi_socket)
       {
         argv.emplace_back ("-M");
         argv.emplace_back (std::to_string (description.shm_size));
         argv.emplace_back ("-S");
         argv.emplace_back (gpi_socket->string());
       }
       if (description.socket)
       {
         argv.emplace_back ("-N");
         argv.emplace_back (std::to_string (description.socket.get()));
       }
       if (!app_path.empty())
       {
         argv.emplace_back ("-L");
         argv.emplace_back (fhg::util::join (app_path, ":"));
       }
       argv.emplace_back ("-H");
       argv.emplace_back (sdpa_home.string());

       startups.emplace_back
         ( std::async ( std::launch::async
                      , [&script_start_drts, host, argv]
                        {
                          rexec ( host
                                , script_start_drts.string()
                                + " " + fhg::util::join (argv, " ")
                                );
                        }
                      )
         );

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
        stop_drts_on_failure (boost::filesystem::path const& state_dir)
          : _state_dir (state_dir)
        {}
        ~stop_drts_on_failure()
        {
          if (!_successful)
          {
            shutdown (_state_dir);
          }
        }
        void startup_successful()
        {
          _successful = true;
        }
        boost::filesystem::path const& _state_dir;
        bool _successful {false};
      } stop_drts_on_failure = {state_dir};

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
                  ( master
                  , "--startup-messages-pipe"
                  , "OKAY"
                  , sdpa_home / "bin" / "orchestrator"
                  , {"-u", "0", "-n", "orchestrator"}
                  , { {"FHGLOG_to_server", log_host + ":" + std::to_string (log_port)}
                    , {"FHGLOG_level", verbose ? "TRACE" : "INFO"}
                    , {"FHGLOG_to_file", (state_dir / "log" / "orchestrator.log").string()}
                    }
                  , sdpa_home
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
                              ( host
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
                              , sdpa_home
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
                                  , state_dir
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
                                       , std::vector<pid_t> const& pids
                                       )
      {
        if (!pids.empty())
        {
          std::cout << "terminating " << name << " on " << host
                    << ": " << fhg::util::join (pids, " ") << "\n";
          rexec (host, "kill -TERM " + fhg::util::join (pids, " "));
        }
      }

      void terminate_all_processes_of_a_kind
        ( boost::filesystem::path const& state_dir
        , std::string const& kind
        , std::vector<std::string> hosts
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
                , [kind, host, pidfiles]()
                  {
                    std::vector<int> pids;
                    for (boost::filesystem::path const& pidfile : pidfiles)
                    {
                      pids.emplace_back ( boost::lexical_cast<pid_t>
                                            (fhg::util::read_file (pidfile))
                                        );
                    }
                    terminate_processes_on_host (kind, host, pids);
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
        terminate_all_processes_of_a_kind (state_dir, "drts-kernel", hosts);
      }
      if (components.get() & components_type::agent)
      {
        terminate_all_processes_of_a_kind (state_dir, "agent", hosts);
      }
      if (components.get() & components_type::vmem)
      {
        terminate_all_processes_of_a_kind (state_dir, "vmem", hosts);
      }
      if (components.get() & components_type::orchestrator)
      {
        boost::filesystem::remove (state_dir / "orchestrator.host");
        boost::filesystem::remove (state_dir / "orchestrator.port");
        boost::filesystem::remove (state_dir / "orchestrator.rpc.host");
        boost::filesystem::remove (state_dir / "orchestrator.rpc.port");

        terminate_all_processes_of_a_kind (state_dir, "orchestrator", hosts);
      }
    }

    void shutdown (boost::filesystem::path const& state_dir)
    {
      shutdown (state_dir, boost::none, {});

      boost::filesystem::remove (state_dir / "processes");
      boost::filesystem::remove (state_dir / "nodefile");
    }
  }
}
