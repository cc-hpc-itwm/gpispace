// mirko.rahn@itwm.fraunhofer.de

#include <drts/system.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace gspc
{
  void system (std::string const& command, std::string const& description)
  {
    if (int ec = std::system (command.c_str()) != 0)
    {
      throw std::runtime_error
        (( boost::format
           ("Could not '%3%': error code '%1%', command was '%2%'")
         % ec
         % command
         % description
         ).str()
        );
    };
  }
}
