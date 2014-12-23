// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/revision.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

int main (int argc, char** argv)
{
  try
  {
    std::string const command_description
      ( "help\t\n"
        "version\t\n"
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
