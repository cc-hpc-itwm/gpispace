// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <drts/drts.hpp>

#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>

#include <string>
#include <unordered_map>

namespace test
{
  class make_net;
  class make_net_lib_install;

  class make
  {
  public:
    boost::filesystem::path pnet() const;

  private:
    friend class make_net;
    friend class make_net_lib_install;

    make (std::string const& main);

    make (make const&) = delete;
    make& operator= (make const&) = delete;
    make (make&&) = delete;
    make& operator= (make&&) = delete;

    std::string const _main;
    fhg::util::temporary_path const _build_directory;

    boost::filesystem::path build_directory() const
    {
      return _build_directory;
    }
  };

  class make_net : public make
  {
  public:
    make_net ( gspc::installation const&
             , std::string const& main
             , boost::filesystem::path const& source_directory
             );
  };

  class make_net_lib_install : public make
  {
  public:
    make_net_lib_install
      ( gspc::installation const&
      , std::string const& main
      , boost::filesystem::path const& source_directory
      , boost::filesystem::path const& lib_destdir
      , std::unordered_map<std::string, std::string> const& make_options
        = std::unordered_map<std::string, std::string> {}
      );
  };
}
