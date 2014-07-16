// mirko.rahn@itwm.fraunhofer.de

#include <drts/drts.hpp>
#include <drts/system.hpp>

#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>

#include <boost/format.hpp>

#include <sstream>

namespace gspc
{
  namespace validators = fhg::util::boost::program_options;

  namespace options
  {
    namespace name
    {
      constexpr char const* const log_host {"log-host"};
      constexpr char const* const log_port {"log-port"};
      constexpr char const* const gui_host {"gui-host"};
      constexpr char const* const gui_port {"gui-port"};
    }

    boost::program_options::options_description logging()
    {
      boost::program_options::options_description logging ("Logging");

      logging.add_options()
        ( name::log_host
        , boost::program_options::value<validators::nonempty_string>()
        ->required()
        , "name of log host"
        )
        ( name::log_port
        , boost::program_options::value
          <validators::positive_integral<unsigned short>>()->required()
        , "port on log-host to log to"
        )
        ( name::gui_host
        , boost::program_options::value<validators::nonempty_string>()
          ->required()
        , "name of gui host"
        )
        ( name::gui_port
        , boost::program_options::value
          <validators::positive_integral<unsigned short>>()->required()
        , "port on gui-host to send to"
        )
        ;

      return logging;
    }
  }

  scoped_runtime_system::scoped_runtime_system
    ( std::string const& boot_options
    , boost::program_options::variables_map const& vm
    , boost::filesystem::path const& gspc_home
    , boost::filesystem::path const& state_directory
    )
      : _gspc_home (gspc_home)
      , _state_directory (state_directory)
  {
    std::string const log_host
      (vm[options::name::log_host].as<validators::nonempty_string>());
    unsigned short const log_port
      ( vm[options::name::log_port]
      . as<validators::positive_integral<unsigned short>>()
      );
    std::string const gui_host
      (vm[options::name::gui_host].as<validators::nonempty_string>());
    unsigned short const gui_port
      ( vm[options::name::gui_port]
      . as<validators::positive_integral<unsigned short>>()
      );

    std::ostringstream command_boot;

    command_boot
      << (_gspc_home / "bin" / "sdpa")
      << " boot"
      << " -l " << log_host << ":" << log_port
      << " -g " << gui_host << ":" << gui_port
      << boot_options;

    system (command_boot.str(), "start runtime system");
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
