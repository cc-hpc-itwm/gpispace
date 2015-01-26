// mirko.rahn@itwm.fraunhofer.de

#include <test/make.hpp>

#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <boost/format.hpp>

#include <sstream>
#include <stdexcept>

namespace test
{
  make::make ( gspc::installation const& installation
             , std::string const& main
             , boost::filesystem::path const& source_directory
             , std::unordered_map<std::string, std::string> const& make_options
             , std::string const& make_targets
             )
    : _build_directory ( boost::filesystem::temp_directory_path()
                       / boost::filesystem::unique_path()
                       )
  {
    std::ostringstream command;

    command
      << "make -f "
      << (installation.gspc_home() / "share" / "sdpa" / "make" / "common.mk")
      << " SDPA_HOME=" << installation.gspc_home()
      << " BOOST_ROOT=" << (installation.gspc_home() / "external" / "boost")
      << " BUILDDIR=" << build_directory()
      << " MAIN=" << main
      ;

    for (std::pair<std::string, std::string> const& options : make_options)
    {
      command << " " << options.first << "=" << options.second;
    }

    command
      << " -C " << source_directory
      << " " << make_targets
      ;

    if (int ec = fhg::util::system_with_blocked_SIGCHLD (command.str().c_str()))
    {
      throw std::runtime_error
        (( boost::format ("Could not run '%1%': error code '%2%'")
         % command.str()
         % ec
         ).str()
        );
    }
  }
}
