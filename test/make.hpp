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
    //! \todo eliminate, at the moment needed because make install
    //! does not copy the pnet files
    boost::filesystem::path build_directory() const
    {
      return _build_directory;
    }

  private:
    friend class make_net;
    friend class make_net_lib_install;

    make ( gspc::installation const&
         , std::string const& main
         , boost::filesystem::path const& source_directory
         , std::unordered_map<std::string, std::string> const& make_options
         , std::string const& make_targets
         );

    make (make const&) = delete;
    make& operator= (make const&) = delete;
    make (make&&) = delete;
    make& operator= (make&&) = delete;

    fhg::util::temporary_path const _build_directory;
  };

  class make_net : public make
  {
  public:
    make_net ( gspc::installation const& installation
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
  };

  class make_net_lib_install : public make
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

  public:
    make_net_lib_install
      ( gspc::installation const& installation
      , std::string const& main
      , boost::filesystem::path const& source_directory
      , boost::filesystem::path const& lib_destdir
      , std::unordered_map<std::string, std::string> const& make_options
        = std::unordered_map<std::string, std::string> {}
      )
        : make ( installation
               , main
               , source_directory
               , add_lib_destdir (make_options, lib_destdir)
               , "net lib install"
               )
    {}
  };
}
