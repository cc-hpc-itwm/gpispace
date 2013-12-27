#include "read_file.hpp"

#include <fstream>
#include <stdexcept>
#include <iterator>

namespace fhg
{
  namespace util
  {
    std::string read_file (std::string const &path)
    {
      std::ifstream ifs (path.c_str (), std::ifstream::binary);
      if (not ifs)
      {
        throw std::runtime_error ("could not open: " + path);
      }
      ifs >> std::noskipws;

      return std::string ( std::istream_iterator<char> (ifs)
                         , std::istream_iterator<char>()
                         );
    }
  }
}
