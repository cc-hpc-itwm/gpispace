// bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_TEST_UTILS_HPP
#define SDPA_TEST_UTILS_HPP

#include <string>
#include <fstream>
#include <sstream>

namespace utils
{
  std::string require_and_read_file (std::string filename)
  {
    std::ifstream f (filename.c_str());
    BOOST_REQUIRE (f.is_open());

    std::ostringstream os;
    char c;
    while (f.get (c)) os << c;
    f.close();

    return os.str();
  }
}

#endif
