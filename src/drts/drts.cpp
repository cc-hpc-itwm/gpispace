// mirko.rahn@itwm.fraunhofer.de

#include <drts/drts.hpp>
#include <drts/system.hpp>

#include <boost/format.hpp>

namespace gspc
{
  scoped_runtime_system::scoped_runtime_system
    ( std::string const& command_boot
    , boost::filesystem::path const& gspc_home
    , boost::filesystem::path const& state_directory
    )
      : _gspc_home (gspc_home)
      , _state_directory (state_directory)
  {
    system (command_boot, "start runtime system");
  }
  scoped_runtime_system::~scoped_runtime_system()
  {
    system ( ( boost::format ("%1% -s %2% stop")
             % (_gspc_home / "bin" / "sdpa")
             % _state_directory
             ).str()
           , "stop runtime system"
           );
  }
}
