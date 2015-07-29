// mirko.rahn@itwm.fraunhofer.de

#include <test/make.hpp>

#include <util-generic/join.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <boost/format.hpp>

#include <exception>
#include <sstream>
#include <stdexcept>

namespace test
{
  make::make (std::string const& main)
    : _main (main)
    , _build_directory ( boost::filesystem::temp_directory_path()
                       / boost::filesystem::unique_path()
                       )
  {}

  boost::filesystem::path make::pnet() const
  {
    return build_directory() / (_main + ".pnet");
  }

  make_net::make_net ( gspc::installation const& installation
                     , std::string const& main
                     , boost::filesystem::path const& source_directory
                     )
    : make (main)
  {
    std::ostringstream command;

    command
      << installation.pnet_compiler()
      << " -I " << installation.workflow_library()
      << " -i " << (source_directory / (main + ".xpnet"))
      << " -o " << pnet()
      << " --gen-cxxflags=-O3"
      ;

    fhg::util::system_with_blocked_SIGCHLD (command.str());
  }

  make_net_lib_install::make_net_lib_install
    ( gspc::installation const& installation
    , std::string const& main
    , boost::filesystem::path const& source_directory
    , boost::filesystem::path const& lib_destdir
    , std::unordered_map<std::string, std::string> const& make_options
    )
      : make (main)
  {
    {
      std::ostringstream command;

      command
        << "make -f "
        << (installation.gspc_home() / "share" / "sdpa" / "make" / "common.mk")
        << " SDPA_HOME=" << installation.gspc_home()
        << " BUILDDIR=" << build_directory()
        << " MAIN=" << main
        ;

      for (std::pair<std::string, std::string> const& options : make_options)
      {
        command << " " << options.first << "=" << options.second;
      }

      command
        << " -C " << source_directory
        << " net gen"
        ;

      fhg::util::system_with_blocked_SIGCHLD (command.str());
    }

    {
      std::ostringstream command;

      command
        << "make "
        << " SDPA_HOME=" << installation.gspc_home()
        << " BOOST_ROOT=" << installation.boost_root()
        << " LIB_DESTDIR=" << lib_destdir
        << " -C " << ( make_options.count ("GEN")
                     ? boost::filesystem::path (make_options.at ("GEN"))
                     : (build_directory() / "gen")
                     )
        << " install"
        ;

      fhg::util::system_with_blocked_SIGCHLD (command.str());
    }
  }
}
