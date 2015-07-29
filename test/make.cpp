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
  make::make ( gspc::installation const& installation
             , std::string const& main
             , boost::filesystem::path const& source_directory
             , std::unordered_map<std::string, std::string> const& make_options
             , std::string const& make_targets
             )
    : _main (main)
    , _build_directory ( boost::filesystem::temp_directory_path()
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

    fhg::util::system_with_blocked_SIGCHLD (command.str());
  }

  boost::filesystem::path make::pnet() const
  {
    return build_directory() / (_main + ".pnet");
  }

  make_net::make_net ( gspc::installation const& installation
                     , std::string const& main
                     , boost::filesystem::path const& source_directory
                     )
    : make ( installation
           , main
           , source_directory
           , std::unordered_map<std::string, std::string> {}
           , "net"
           )
  {}

  namespace
  {
    std::unordered_map<std::string, std::string> add_lib_destdir
      ( std::unordered_map<std::string, std::string> const& make_options
      , boost::filesystem::path const& lib_destdir
      )
    {
      std::unordered_map<std::string, std::string> options (make_options);
      if (!options.emplace ("LIB_DESTDIR", lib_destdir.string()).second)
      {
        throw std::invalid_argument
          (( boost::format ("Multiple definitions of LIB_DESTDIR:"
                           " Found %1% as parameter and %2% in the make options"
                           )
           % lib_destdir
           % make_options.at ("LIB_DESTDIR")
           ).str()
          )
          ;
      }
      return options;
    }
  }

  make_net_lib_install::make_net_lib_install
    ( gspc::installation const& installation
    , std::string const& main
    , boost::filesystem::path const& source_directory
    , boost::filesystem::path const& lib_destdir
    , std::unordered_map<std::string, std::string> const& make_options
    )
      : make ( installation
             , main
             , source_directory
             , add_lib_destdir (make_options, lib_destdir)
             , "net lib install"
             )
  {}
}
