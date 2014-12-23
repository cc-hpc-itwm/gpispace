// bernd.loerwald@itwm.fraunhofer.de

#include <drts/private/startup_and_shutdown.hpp>

#include <fhg/revision.hpp>
#include <fhg/syscall.hpp>
#include <fhg/util/boost/program_options/validators/is_directory_if_exists.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_file.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/executable_path.hpp>
#include <fhg/util/hostname.hpp>
#include <fhg/util/join.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <fhgcom/peer_info.hpp>

#include <sdpa/client.hpp>
#include <sdpa/job_states.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include <chrono>
#include <fstream>

namespace
{
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

  std::ostream &operator<< ( std::ostream &stream
                           , std::chrono::system_clock::time_point const& tp
                           )
  {
    std::time_t const tp_c (std::chrono::system_clock::to_time_t (tp));
    char mbstr[100];
    if (!std::strftime (mbstr, sizeof (mbstr), "%c", std::localtime (&tp_c)))
    {
      throw std::length_error ("strftime: out of space");
    }
    return stream << mbstr;
  }
}

//! \todo unify with gspc::client
void submit ( boost::filesystem::path const& state_dir
            , boost::filesystem::path const& net
            , boost::filesystem::path const& output
            )
{
  fhg::com::host_t const orchestrator_host
    (fhg::util::read_file (state_dir / "orchestrator.host"));
  fhg::com::port_t const orchestrator_port
    (fhg::util::read_file (state_dir / "orchestrator.port"));

  std::cout << "I: submitting file: " << net << "\n";

  boost::asio::io_service io_service;
  sdpa::client::Client sdpac (orchestrator_host, orchestrator_port, io_service);

  sdpa::job_id_t jobid (sdpac.submitJob (fhg::util::read_file (net)));
  std::cout << "  + job (" << jobid << ")\n";

  std::chrono::time_point<std::chrono::system_clock> const start
    (std::chrono::system_clock::now());

  std::cout << "starting at: " << start << "\n"
            << "waiting for job to return..." << std::endl;

  sdpa::client::job_info_t job_info;
  sdpa::status::code const status
    (sdpac.wait_for_terminal_state (jobid, job_info));

  std::chrono::time_point<std::chrono::system_clock> const end
    (std::chrono::system_clock::now());

  std::cout << "stopped at: " << end << "\n"
            << "execution time: "
            << std::chrono::duration<float, std::chrono::seconds::period>
                 (end - start).count()
            << " s" << std::endl;

  if (sdpa::status::FAILED == status)
  {
    std::cout << "failed: "
              << "error-message := " << job_info.error_message
              << std::endl;
  }

  std::cout << "    " << jobid << " -> " << sdpa::status::show (status) << "\n";

  {
    std::ofstream ofs (output.string());
    ofs << sdpac.retrieveResults (jobid);
  }
  std::cout << "stored results in: " << output << "\n";

  sdpac.deleteJob (jobid);
  std::cout << "  - job (" << jobid << ")\n";

  if (sdpa::status::FAILED == status)
  {
    throw std::runtime_error ("job failed");
  }
  else if (sdpa::status::CANCELED == status)
  {
    throw std::runtime_error ("job canceled");
  }
}

namespace
{
  std::vector<std::string> chop_first (std::vector<std::string> vec)
  {
    if (!vec.empty())
    {
      vec.erase (vec.begin());
    }
    return vec;
  }

  std::pair<boost::filesystem::path, std::vector<std::string>>
    get_state_dir_and_extra_args (int argc, char** argv)
  {
    if (argc < 2 || argv[0] != std::string ("-s"))
    {
      throw std::invalid_argument ("missing state dir: -s <path>");
    }
    return std::pair<boost::filesystem::path, std::vector<std::string>>
      ( boost::filesystem::canonical (argv[1])
      , std::vector<std::string> (argv + 2, argv + argc)
      );
  }

  template<typename T>
    T get (boost::program_options::variables_map const& vm, const char* option)
  {
    return vm.at (option).as<T>();
  }

  template<typename T, typename U = T>
    boost::optional<U> get_or_none
      ( boost::program_options::variables_map const& vm
      , const char* option
      )
  {
    return vm.count (option)
      ? boost::optional<U> (vm.at (option).as<T>())
      : boost::optional<U>();
  }

  std::pair<std::string, unsigned short> require_host_port (std::string const& s)
  {
    const boost::tokenizer<boost::char_separator<char>> tok
      (s, boost::char_separator<char> (":"));


    const std::vector<std::string> vec (tok.begin(), tok.end());
    if (vec.size() != 2)
    {
      throw std::invalid_argument ("host and port not of format 'host:port'");
    }

    return {vec[0], boost::lexical_cast<unsigned short> (vec[1])};
  }
}

int main (int argc, char** argv)
{
  try
  {
    boost::filesystem::path const SDPA_HOME
      (fhg::util::executable_path().parent_path().parent_path());

    std::string const command_description
      ( "help\t\n"
        "version\t\n"
        "gui <gui port> <log port>\t\n"
        "selftest\tperform a self-test\n"
        "boot [options]\tboot the runtime system\n"
        "stop [vmem|orchestrator|agent|drts [host...]]\tstop component(s) on hosts\n"
        "submit <file>\tsubmit a job"
      );

    if (argc < 2)
    {
      throw std::invalid_argument ("no command given:\n" + command_description);
    }

    //! \todo Use Boost.ProgramOptions instead of handcrafted parsers
    std::string const command (argv[1]);

    if (command == "help")
    {
      std::cout << "usage: " << argv[0] << " [options]\n"
                << command_description << "\n";
    }
    else if (command == "version")
    {
      std::cout << fhg::project_info ("GPI-Space");
    }
    else if (command == "gui")
    {
      if (argc != 4)
      {
        std::invalid_argument ("usage: 'sdpa gui <gui port> <log port>");
      }

      system ( ( (SDPA_HOME / "bin" / "sdpa-gui").string()
               + argv[2] + " " + argv[3]
               ).c_str()
             , "sdpa-gui implementation"
             );
    }
    else if (command == "stop")
    {
      std::pair<boost::filesystem::path, std::vector<std::string>> const
        state_dir_and_extra_args (get_state_dir_and_extra_args (argc - 2, argv + 2));
      if (!state_dir_and_extra_args.second.empty())
      {
        if (state_dir_and_extra_args.second.front() == "vmem")
        {
          fhg::drts::shutdown ( state_dir_and_extra_args.first
                              , fhg::drts::components_type::vmem
                              , chop_first (state_dir_and_extra_args.second)
                              );
        }
        else if (state_dir_and_extra_args.second.front() == "orchestrator")
        {
          fhg::drts::shutdown ( state_dir_and_extra_args.first
                              , fhg::drts::components_type::orchestrator
                              , chop_first (state_dir_and_extra_args.second)
                              );
        }
        else if (state_dir_and_extra_args.second.front() == "agent")
        {
          fhg::drts::shutdown ( state_dir_and_extra_args.first
                              , fhg::drts::components_type::agent
                              , chop_first (state_dir_and_extra_args.second)
                              );
        }
        else if (state_dir_and_extra_args.second.front() == "drts")
        {
          fhg::drts::shutdown ( state_dir_and_extra_args.first
                              , fhg::drts::components_type::worker
                              , chop_first (state_dir_and_extra_args.second)
                              );
        }
        else
        {
          throw std::invalid_argument
            ("bad component: " + state_dir_and_extra_args.second.front());
        }
      }
      else
      {
        fhg::drts::shutdown (state_dir_and_extra_args.first, boost::none, {});
      }
    }
    else if (command == "submit")
    {
      std::pair<boost::filesystem::path, std::vector<std::string>> const
        state_dir_and_extra_args (get_state_dir_and_extra_args (argc - 2, argv + 2));
      if (state_dir_and_extra_args.second.size() < 1)
      {
        throw std::invalid_argument ("missing argument: net");
      }

      submit ( state_dir_and_extra_args.first
             , state_dir_and_extra_args.second[0] == "-"
             ? "/dev/stdin"
             : state_dir_and_extra_args.second[0]
             , state_dir_and_extra_args.second.size() >= 2
             ? state_dir_and_extra_args.second[1]
             : "/dev/null"
             );
    }
    else if (command == "boot")
    {
      boost::program_options::options_description options;
      options.add_options()
        ("help,h", "print this help")
        ( "verbose,v"
        , boost::program_options::value<bool>()
          ->implicit_value (true)->default_value (false)
        , "let components be verbose"
        )
        ( "nodefile,f"
        , boost::program_options::value
            <fhg::util::boost::program_options::nonempty_file>()->required()
        , "node file to use"
        )
        ( "state,s"
        , boost::program_options::value
            <fhg::util::boost::program_options::is_directory_if_exists>()
          ->required()
        , "directory to use for state"
        )
        ( "gpi-mem,m"
        , boost::program_options::value<std::size_t>()
        , "amount of gpi memory to use (bytes)"
        )
        ( "worker-description,C"
        , boost::program_options::value<std::vector<std::string>>()
          ->default_value (std::vector<std::string>(), "{}")
        , "<capability>[+capability...][#socket][:N[xM][,shm]]\n"
          "\tINIT:1x1,0\n"
          "\t\texactly *one* INIT worker\n"
          "\tLOAD:2,16777216\n"
          "\t\t2 LOAD worker/node\n"
          "\tWRITE:2x4,16777216\n"
          "\t\t2 WRITE workers/node but on at most 4 nodes\n"
          "\tWRITE+LOAD:2\n"
          "\t\t2 workers/node with capabilities WRITE *and* LOAD\n"
        )
        ( "def-num-proc,p"
        , boost::program_options::value<std::size_t>()->default_value (1)
        , "default number of processes per node for a worker"
        )
        ( "gpi-disabled,M"
        , boost::program_options::value<bool>()
          ->implicit_value (true)->default_value (false)->zero_tokens()
        , "disable use of the memory layer"
        )
        ( "app-path,A"
        , boost::program_options::value<std::vector<boost::filesystem::path>>()
          ->default_value (std::vector<boost::filesystem::path>(), "{}")
        , "add a path to the list of application search paths"
        )
        ( "dont-delete-logfiles,d"
        , boost::program_options::value<bool>()
          ->implicit_value (true)->default_value (false)
        , "do not delete logfiles"
        )
        ( "number-of-groups,G"
        , boost::program_options::value
            <fhg::util::boost::program_options::positive_integral<std::size_t>>()
          ->default_value (1)
        , "number of segments"
        )
        ( "gpi-socket,y"
        , boost::program_options::value
            <fhg::util::boost::program_options::nonexisting_path>()
        , "Socket file to communicate with the virtual memory manager"
        )
        ( "vmem-startup-timeout,T"
        , boost::program_options::value<std::chrono::seconds::rep>()
        , "timeout in seconds for virtual memory manager to start"
        )
        ( "vmem-port,P"
        , boost::program_options::value<unsigned short>()
        , "use this port for the virtual memory manager"
        )
        ( "log-url,l"
        , boost::program_options::value<std::string>()
        , "log url"
        )
        ( "gui-url,l"
        , boost::program_options::value<std::string>()
        , "gui url"
        )
        ;
      boost::program_options::positional_options_description positional_options;
      positional_options.add ("worker-description", -1);

      boost::program_options::variables_map vm;
      boost::program_options::store
        ( boost::program_options::command_line_parser (argc - 1, argv + 1)
          .options (options).positional (positional_options).run()
        , vm
        );

      if (vm.count ("help"))
      {
        std::cout << "usage: " << argv[0] << " boot [options]\n"
                  << options << "\n";
        return 0;
      }

      boost::program_options::notify (vm);

      if (!get<bool> (vm, "gpi-disabled"))
      {
        if (!vm.count ("vmem-startup-timeout"))
        {
          throw std::invalid_argument
            ("vmem timeout is required (--vmem-startup-timeout)");
        }
        if (!vm.count ("vmem-port"))
        {
          throw std::invalid_argument ("vmem port is required (--vmem-port)");
        }
        if (!vm.count ("gpi-socket"))
        {
          throw std::invalid_argument ("vmem socket is required (--gpi-socket)");
        }
      }

      unsigned short const default_log_port
        ((65535 - 30000 + fhg::syscall::getuid() * 2) % 65535 + 1024);
      unsigned short const default_gui_port (default_log_port + 1);
      boost::optional<std::string> const log_url
        (get_or_none<std::string> (vm, "log-url"));
      boost::optional<std::string> const gui_url
        (get_or_none<std::string> (vm, "gui-url"));

      std::vector<fhg::drts::worker_description> worker_descriptions;
      for ( std::string const& description
          : get<std::vector<std::string>> (vm, "worker-description")
          )
      {
        worker_descriptions.emplace_back
          ( fhg::drts::parse_capability
              (get<std::size_t> (vm, "def-num-proc"), description)
          );
      }

      fhg::util::signal_handler_manager signal_handler_manager;

      fhg::drts::startup
        ( gui_url ? require_host_port (gui_url.get()).first : fhg::util::hostname()
        , gui_url ? require_host_port (gui_url.get()).second : default_gui_port
        , log_url ? require_host_port (log_url.get()).first : fhg::util::hostname()
        , log_url ? require_host_port (log_url.get()).second : default_log_port
        , !get<bool> (vm, "gpi-disabled")
        , get<bool> (vm, "verbose")
        , get_or_none < fhg::util::boost::program_options::nonexisting_path
                      , boost::filesystem::path
                      > (vm, "gpi-socket")
        , get<std::vector<boost::filesystem::path>> (vm, "app-path")
        , SDPA_HOME
        , get<fhg::util::boost::program_options::positive_integral<std::size_t>>
            (vm, "number-of-groups")
        , boost::filesystem::canonical
            ( get<fhg::util::boost::program_options::nonempty_file>
                (vm, "nodefile")
            )
        , get<fhg::util::boost::program_options::is_directory_if_exists>
            (vm, "state")
        , !get<bool> (vm, "dont-delete-logfiles")
        , signal_handler_manager
        , get_or_none<std::size_t> (vm, "gpi-mem")
        , get_or_none<std::chrono::seconds::rep, std::chrono::seconds>
            (vm, "vmem-startup-timeout")
        , worker_descriptions
        , get_or_none<unsigned short> (vm, "vmem-port")
        , std::chrono::minutes (1)
        );
    }
    else if (command == "selftest")
    {
      std::pair<boost::filesystem::path, std::vector<std::string>> const
        state_dir_and_extra_args (get_state_dir_and_extra_args (argc - 2, argv + 2));
      system ( ( (SDPA_HOME / "libexec" / "sdpa" / "scripts" / "sdpa-selftest").string()
               + " -s \"" + state_dir_and_extra_args.first.string()
               + "\" -H \"" + SDPA_HOME.string() + "\" "
               + fhg::util::join (state_dir_and_extra_args.second, " ")
               ).c_str()
             , "sdpa-selftest implementation"
             );
    }
    else
    {
      throw std::invalid_argument
        ("unknown command: " + command + ", try " + argv[0] + " help");
    }
  }
  catch (std::runtime_error const& ex)
  {
    std::cerr << "E: " << ex.what() << std::endl;
    return 1;
  }
}
