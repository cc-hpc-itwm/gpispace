// mirko.rahn@itwm.fraunhofer.de

#ifndef TEST_SOURCE_DIRECTORY_HPP
#define TEST_SOURCE_DIRECTORY_HPP

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace test
{
  namespace options
  {
    namespace name
    {
      constexpr char const* const source_directory {"source-directory"};
    }

    boost::program_options::options_description source_directory();
  }

  boost::filesystem::path source_directory
    (boost::program_options::variables_map const&);
}

#endif
