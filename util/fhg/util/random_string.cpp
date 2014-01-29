// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/random_string.hpp>

#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <algorithm>
#include <string>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    std::string const random_string_of (std::string const& chars)
    {
      if (chars.empty())
      {
        throw std::runtime_error ("random_string_of (empty_string)");
      }

      boost::random::random_device rng;

      int const length
        (boost::random::uniform_int_distribution<> (0, 1 << 10) (rng));

      boost::random::uniform_int_distribution<> index (0, chars.size() - 1);

      std::string s;

      for (int i = 0; i < length; ++i)
      {
        s.push_back (chars [index (rng)]);
      }

      return s;
    }

    std::string const random_string()
    {
      std::string chars;

      for (int i = 0; i < 256; ++i)
      {
        chars.push_back (char (i));
      }

      return random_string_of (chars);
    }

    std::string const random_string_without (std::string const& except)
    {
      std::string chars;

      for (int i = 0; i < 256; ++i)
      {
        if (std::find (except.begin(), except.end(), char (i)) == except.end())
        {
          chars.push_back (char (i));
        }
      }

      return random_string_of (chars);
    }
  }
}
