// mirko.rahn@itwm.fraunhofer.de

#ifndef TEST_SCOPED_STATE_DIRECTORY_HPP
#define TEST_SCOPED_STATE_DIRECTORY_HPP

#include <fhg/util/temporary_path.hpp>

#include <boost/program_options.hpp>

namespace test
{
  class scoped_state_directory
  {
  public:
    scoped_state_directory (boost::program_options::variables_map&);

  private:
    fhg::util::temporary_path const _temporary_path;
  };
}

#endif
