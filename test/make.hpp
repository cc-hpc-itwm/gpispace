// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <drts/drts.hpp>

#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>

#include <string>
#include <unordered_map>

namespace test
{
  class make
  {
  public:
    make ( gspc::installation const&
         , std::string const& main
         , boost::filesystem::path const& source_directory
         , std::unordered_map<std::string, std::string> const& make_options
         , std::string const& make_targets
         );

    //! \todo eliminate, at the moment needed because make install
    //! does not copy the pnet files
    boost::filesystem::path build_directory() const
    {
      return _build_directory;
    }

    make (make const&) = delete;
    make& operator= (make const&) = delete;
    make (make&&) = delete;
    make& operator= (make&&) = delete;

  private:
    fhg::util::temporary_path const _build_directory;
  };
}
