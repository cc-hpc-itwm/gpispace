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
             , boost::optional<boost::filesystem::path> const& lib_destdir
             , option::options const& options
             )
    : _main (main)
    , _build_directory ( boost::filesystem::temp_directory_path()
                       / boost::filesystem::unique_path()
                       )
    , _pnet ( static_cast<boost::filesystem::path> (_build_directory)
            / (_main + ".pnet")
            )
  {
    boost::filesystem::path const wrapper_directory
      (static_cast<boost::filesystem::path> (_build_directory) / "gen");

    {
      std::ostringstream command;

      command
        << installation.pnet_compiler()
        << option::include (installation.workflow_library())
        << option::generic ("input", source_directory / (_main + ".xpnet"))
        << option::generic ("output", pnet())
        << option::gen::cxx_flag ("-O3")
        << options
        ;

      if (!!lib_destdir)
      {
        command << option::generic ("path-to-cpp", wrapper_directory);
      }

      fhg::util::system_with_blocked_SIGCHLD (command.str());
    }

    if (!!lib_destdir)
    {
      std::ostringstream command;

      command
        << "make "
        << " SDPA_HOME=" << installation.gspc_home()
        << " BOOST_ROOT=" << installation.boost_root()
        << " LIB_DESTDIR=" << lib_destdir.get()
        << " -C " << wrapper_directory
        << " install"
        ;

      fhg::util::system_with_blocked_SIGCHLD (command.str());
    }
  }

  namespace option
  {
    generic::generic (std::string const& key, char const* const value)
      : generic (key, std::string (value))
    {}
    generic::generic (std::string const& key, std::string const& value)
      : _key (key)
      , _value (value)
    {}
    generic::generic (std::string const& key, boost::format const& format)
      : generic (key, format.str())
    {}
    generic::generic ( std::string const& key
                     , boost::filesystem::path const& path
                     )
      : generic (key, boost::format ("%1%") % path)
    {}
    std::ostream& generic::operator() (std::ostream& os) const
    {
      //! \todo quoting
      return os << " --" << _key << "=" << _value ;
    }
    std::ostream& options::operator() (std::ostream& os) const
    {
      for (auto&& option : _options)
      {
        os << *option;
      }

      return os;
    }

    namespace gen
    {
      cxx11::cxx11()
        : cxx_flag ("--std=c++11")
      {}
      include::include (boost::filesystem::path const& path)
        : cxx_flag (boost::format ("'-I %1%'") % path)
      {}
      link::link (boost::filesystem::path const& path)
        : ld_flag (path.string())
      {}
      library_path::library_path (boost::filesystem::path const& path)
        : ld_flag (boost::format ("'-L %1%'") % path)
      {}
    }
  }
}
