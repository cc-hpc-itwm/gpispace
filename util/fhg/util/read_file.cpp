#include "read_file.hpp"

#include <boost/format.hpp>

#include <fstream>
#include <stdexcept>
#include <iterator>

namespace fhg
{
  namespace util
  {
    std::string read_file (boost::filesystem::path const &path)
    {
      std::ifstream ifs (path.string().c_str (), std::ifstream::binary);
      if (not ifs)
      {
        throw std::runtime_error
          ((boost::format ("could not open %1%") % path).str());
      }
      ifs >> std::noskipws;

      return std::string ( std::istream_iterator<char> (ifs)
                         , std::istream_iterator<char>()
                         );
    }
  }
}
