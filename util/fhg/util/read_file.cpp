#include "read_file.hpp"

#include <fstream>
#include <stdexcept>

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

      std::filebuf *pbuf = ifs.rdbuf ();

      std::size_t fsize = pbuf->pubseekoff (0, ifs.end, ifs.in);
      pbuf->pubseekpos (0, ifs.in);

      if (fsize != (std::size_t)(-1))
      {
        char *buf = new char[fsize];
        pbuf->sgetn (buf, fsize);

        std::string s (buf, fsize);

        delete [] buf;

        return s;
      }
      else
      {
        throw std::runtime_error ("could not determine file-size: " + path);
      }
    }
  }
}
