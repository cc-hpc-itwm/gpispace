// bernd.loerwald@itwm.fraunhofer.de

#include <drts/private/startup_and_shutdown.hpp>

#include <fhg/revision.hpp>
#include <fhg/util/executable_path.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <fhgcom/peer_info.hpp>

#include <sdpa/client.hpp>
#include <sdpa/job_states.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/format.hpp>

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
