#ifndef _H_UFBMIG_UTIL_RW
#define _H_UFBMIG_UTIL_RW

#include <we/type/value/read.hpp>

#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

namespace ufbmig
{
  namespace util
  {
    namespace serialize
    {
      template<typename T>
      inline void write (const std::string & filename, const T & x)
      {
        std::ofstream f (filename.c_str());

        if (!f.good())
          {
            std::ostringstream oss;

            oss << "serialize::write to file " << filename << " failed";

            throw std::runtime_error (oss.str());
          }

        f << x;
      }

      template<typename T>
      inline void read (const std::string & filename, T & x)
      {
        std::ifstream f (filename.c_str());

        if (!f.good())
          {
            std::ostringstream oss;

            oss << "serialize::read from file " << filename << " failed";

            throw std::runtime_error (oss.str());
          }

        x = T ( pnet::type::value::read
                ( std::string ( std::istream_iterator<char> (f)
                              , std::istream_iterator<char>()
                              )
                )
              );
      }
    }
  }
}

#endif
