// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <testing/make.hpp>

#include <testing/make_environment.hpp>

#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <boost/version.hpp>

#include <FMT/boost/filesystem/path.hpp>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/std.h>
#include <sstream>

namespace test
{
  //! \todo configure
  namespace
  {
    std::filesystem::path pnet_compiler (gspc::installation const& installation)
    {
      return installation.gspc_home() / "bin" / "pnetc";
    }
    std::filesystem::path workflow_library
      (gspc::installation const& installation)
    {
      return installation.gspc_home() / "share" / "GPISpace" / "xml" / "lib";
    }
  }

  make::make ( gspc::installation const& installation
             , std::string const& main
             , ::boost::filesystem::path const& source_directory
             , std::optional<::boost::filesystem::path> const& lib_destdir
             , option::options const& options
             )
    : _main (main)
    , _build_directory ( ::boost::filesystem::temp_directory_path()
                       / ::boost::filesystem::unique_path()
                       )
    , _pnet ( static_cast<::boost::filesystem::path> (_build_directory)
            / (_main + ".pnet")
            )
  {
    ::boost::filesystem::path const wrapper_directory
      (static_cast<::boost::filesystem::path> (_build_directory) / "gen");

    {
      std::ostringstream command;

      command
        << pnet_compiler (installation)
        << option::include (workflow_library (installation))
        << option::generic ("input", source_directory / (_main + ".xpnet"))
        << option::generic ("output", pnet())
        << option::gen::cxx_flag ("-O3")
        << options
        ;

      if (lib_destdir.has_value())
      {
        command << option::generic ("path-to-cpp", wrapper_directory);
      }

      fhg::util::system_with_blocked_SIGCHLD (command.str());
    }

    if (lib_destdir.has_value())
    {
      std::ostringstream command;

      // The two boost defines are necessary for boost versions >1.61.0 where
      // coroutines have been deprecated, but are still used by ::boost::asio.
      // The second flag is required specifically for boost 1.62.0 where both
      // flags occur due to a typo. This is fixed in later versions.
      command
        << "CXXFLAGS=\"-Wall -Wextra -Werror"
#if BOOST_VERSION == 106200 // 1.62.0
        << " -DBOOST_COROUTINES_NO_DEPRECATION_WARNING -DBOOST_COROUTINE_NO_DEPRECATION_WARNING"
#else // >=1.63.0
        << " -DBOOST_COROUTINES_NO_DEPRECATION_WARNING"
#endif
        << " -DGSPC_WITH_IML=" << GSPC_WITH_IML << "\""
        << " CXX=\"" << cmake_cxx_compiler << "\""
        << " make "
        << " LIB_DESTDIR=" << *lib_destdir
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
    generic::generic ( std::string const& key
                     , ::boost::filesystem::path const& path
                     )
      : generic {key, fmt::format ("{}", path)}
    {}
    generic::generic ( std::string const& key
                     , std::filesystem::path const& path
                     )
      : generic {key, fmt::format ("{}", path)}
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
      include::include (::boost::filesystem::path const& path)
        : cxx_flag {fmt::format ("'-I {}'",  path)}
      {}
      include::include (std::filesystem::path const& path)
        : cxx_flag {fmt::format ("'-I {}'",  path)}
      {}
      link::link (::boost::filesystem::path const& path)
        : ld_flag (path.string())
      {}
      link::link (std::filesystem::path const& path)
        : ld_flag (path.string())
      {}
      library_path::library_path (::boost::filesystem::path const& path)
        : ld_flag {fmt::format ("'-L {}'", path)}
      {}
      library_path::library_path (std::filesystem::path const& path)
        : ld_flag {fmt::format ("'-L {}'", path)}
      {}
    }
  }
}
