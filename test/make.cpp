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
      << " --force-overwrite-file=true"
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
    std::ostringstream command;

    command
      << "make -f "
      << (installation.gspc_home() / "share" / "sdpa" / "make" / "common.mk")
      << " SDPA_HOME=" << installation.gspc_home()
      << " BOOST_ROOT=" << installation.boost_root()
      << " BUILDDIR=" << build_directory()
      << " MAIN=" << main
      << " LIB_DESTDIR=" << lib_destdir
      ;

    for (std::pair<std::string, std::string> const& options : make_options)
    {
      if (options.first == "LIB_DESTDIR")
      {
        throw std::invalid_argument
          (( boost::format ("Multiple definitions of LIB_DESTDIR:"
                           " Found %1% as parameter and %2% in the make options"
                           )
           % lib_destdir
           % options.second
           ).str()
          );
      }

      command << " " << options.first << "=" << options.second;
    }

    command
      << " -C " << source_directory
      << " net lib install"
      ;

     fhg::util::system_with_blocked_SIGCHLD (command.str());
  }
}
