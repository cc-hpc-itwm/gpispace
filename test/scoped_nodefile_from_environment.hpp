// alexander.petry@itwm.fraunhofer.de

#ifndef TEST_SCOPED_NODEFILE_FROM_ENVIRONMENT_HPP
#define TEST_SCOPED_NODEFILE_FROM_ENVIRONMENT_HPP

#include <fhg/util/temporary_file.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace test
{
  class scoped_nodefile_from_environment
  {
  public:
    scoped_nodefile_from_environment
      ( boost::filesystem::path const& shared_directory
      , boost::program_options::variables_map&
      );

  private:
    fhg::util::temporary_file const _temporary_file;
  };
}

#endif
