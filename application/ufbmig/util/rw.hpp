#ifndef _H_UFBMIG_UTIL_RW
#define _H_UFBMIG_UTIL_RW 1

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <fstream>
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

        boost::archive::text_oarchive oa (f);

        oa << x;
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

        boost::archive::text_iarchive oa (f);

        oa >> x;
      }
    } // namespace serialize
  } // namespace util
} // namespace ufbmig

#endif
