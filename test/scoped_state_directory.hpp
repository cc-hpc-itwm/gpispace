// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <fhg/util/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace test
{
  class scoped_state_directory
  {
  public:
    scoped_state_directory
      ( boost::filesystem::path const& shared_directory
      , boost::program_options::variables_map&
      );

  private:
    fhg::util::temporary_path const _temporary_path;
  };
}
