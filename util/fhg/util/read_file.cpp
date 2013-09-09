#include "read_file.hpp"

#include <fstream>

namespace fhg
{
  namespace util
  {
    std::string read_file (std::string const &path)
    {
      std::ifstream ifs (path.c_str (), std::ifstream::binary);
      std::filebuf *pbuf = ifs.rdbuf ();

      std::size_t fsize = pbuf->pubseekoff (0, ifs.end, ifs.in);
      pbuf->pubseekpos (0, ifs.in);

      char *buf = new char[fsize];
      pbuf->sgetn (buf, fsize);

      ifs.close ();

      std::string s (buf, fsize);

      delete [] buf;

      return s;
    }
  }
}
