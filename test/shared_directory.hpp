// mirko.rahn@itwm.fraunhofer.de

#ifndef TEST_SHARED_DIRECTORY_HPP
#define TEST_SHARED_DIRECTORY_HPP

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace test
{
  namespace options
  {
    boost::program_options::options_description shared_directory();
  }

  boost::filesystem::path shared_directory
    (boost::program_options::variables_map const&);
}

#endif
