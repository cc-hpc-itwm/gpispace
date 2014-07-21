// mirko.rahn@itwm.fraunhofer.de

#ifndef TEST_SCOPED_NODEFILE_WITH_LOCALHOST_HPP
#define TEST_SCOPED_NODEFILE_WITH_LOCALHOST_HPP

#include <fhg/util/temporary_file.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace test
{
  class scoped_nodefile_with_localhost
  {
  public:
    scoped_nodefile_with_localhost (boost::program_options::variables_map&);

  private:
    fhg::util::temporary_file const _temporary_file;
  };
}

#endif
