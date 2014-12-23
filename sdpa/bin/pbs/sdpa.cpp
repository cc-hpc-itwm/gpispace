// bernd.loerwald@itwm.fraunhofer.de

#include <drts/private/startup_and_shutdown.hpp>

#include <fhg/revision.hpp>

#include <boost/filesystem.hpp>

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
    std::string const command_description
      ( "help\t\n"
        "version\t\n"
        "stop [vmem|orchestrator|agent|drts [host...]]\tstop component(s) on hosts\n"
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
