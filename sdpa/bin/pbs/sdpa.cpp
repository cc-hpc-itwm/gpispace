// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/revision.hpp>
#include <fhg/util/boost/program_options/validators/existing_directory.hpp>
#include <fhg/util/executable_path.hpp>
#include <fhg/util/join.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <fhgcom/peer_info.hpp>

#include <sdpa/client.hpp>
#include <sdpa/job_states.hpp>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/iterator_range_core.hpp>

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <vector>

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
}

void rexec (std::string const& host, std::string const& command)
{
  std::string const ssh_opts
    ("-q -x -T -n -C -4 -o CheckHostIP=no -o StrictHostKeyChecking=no");
  system ("ssh " + ssh_opts + " " + host + " " + command, "rexec");
}

void terminate_processes_on_host
  (std::string const& name, std::string const& host, std::vector<int> const& pids)
{
  if (!pids.empty())
  {
    std::cout << "terminating " << name << " on " << host
              << ": " << fhg::util::join (pids, " ") << "\n";
    rexec (host, "kill -TERM " + fhg::util::join (pids, " "));
  }
}

void terminate_all_processes_of_a_kind ( boost::filesystem::path const& state_dir
                                       , std::string const& kind
                                       , std::vector<std::string> hosts
                                       )
{
  boost::filesystem::path const processes_dir (state_dir / "processes");

  if (hosts.empty())
  {
    for ( boost::filesystem::directory_entry const& entry
        : boost::make_iterator_range
      (boost::filesystem::directory_iterator (processes_dir), boost::filesystem::directory_iterator())
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
        : boost::make_iterator_range
      (boost::filesystem::directory_iterator (processes_dir / host), boost::filesystem::directory_iterator())
        | boost::adaptors::filtered
            ( [&kind] (boost::filesystem::directory_entry const& entry)
              {
                return entry.status().type() == boost::filesystem::regular_file
                  && entry.path().extension() == ".pid"
                  && entry.path().stem().string().compare (0, kind.size(), kind);
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
                pids.emplace_back (std::stoi (fhg::util::read_file (pidfile)));
              }
              terminate_processes_on_host (kind, host, pids);
              for (boost::filesystem::path const& pidfile : pidfiles)
              {
                boost::filesystem::remove (pidfile);
              }
            }
          )
      );

    if (terminates.size() >= 16)
    {
      for (std::future<void>& terminate : terminates)
      {
        terminate.get();
      }
      terminates.clear();
    }
  }

  for (std::future<void>& terminate : terminates)
  {
    terminate.get();
  }

  for ( boost::filesystem::directory_entry const& entry
      : boost::make_iterator_range
    (boost::filesystem::directory_iterator (processes_dir), boost::filesystem::directory_iterator())
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

void stop ( boost::filesystem::path const& state_dir
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
  if (components.get() & components_type::agent)
  {
    terminate_all_processes_of_a_kind (state_dir, "agent", hosts);
  }
  if (components.get() & components_type::worker)
  {
    terminate_all_processes_of_a_kind (state_dir, "drts-kernel", hosts);
  }
}

namespace
{
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
          stop ( state_dir_and_extra_args.first
               , components_type::vmem
               , chop_first (state_dir_and_extra_args.second)
               );
        }
        else if (state_dir_and_extra_args.second.front() == "orchestrator")
        {
          stop ( state_dir_and_extra_args.first
               , components_type::orchestrator
               , chop_first (state_dir_and_extra_args.second)
               );
        }
        else if (state_dir_and_extra_args.second.front() == "agent")
        {
          stop ( state_dir_and_extra_args.first
               , components_type::agent
               , chop_first (state_dir_and_extra_args.second)
               );
        }
        else if (state_dir_and_extra_args.second.front() == "drts")
        {
          stop ( state_dir_and_extra_args.first
               , components_type::worker
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
        stop (state_dir_and_extra_args.first, boost::none, {});
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
      std::pair<boost::filesystem::path, std::vector<std::string>> const
        state_dir_and_extra_args (get_state_dir_and_extra_args (argc - 2, argv + 2));
      system ( ( (SDPA_HOME / "libexec" / "sdpa" / "scripts" / "start-sdpa").string()
               + " -S \"" + state_dir_and_extra_args.first.string()
               + "\" -H \"" + SDPA_HOME.string() + "\" "
               + fhg::util::join (state_dir_and_extra_args.second, " ")
               ).c_str()
             , "sdpa-boot implementation"
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
