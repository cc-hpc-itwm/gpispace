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
    char random_char_of (std::string const& chars)
    {
      if (chars.empty())
      {
        throw std::runtime_error ("random_char_of (empty_string)");
      }

      boost::random::random_device rng;

      boost::random::uniform_int_distribution<> index (0, chars.size() - 1);

      return chars [index (rng)];
    }

    std::string random_string_of (std::string const& chars)
    {
      boost::random::random_device rng;

      int const length
        (boost::random::uniform_int_distribution<> (0, 1 << 10) (rng));

      std::string s;

      for (int i = 0; i < length; ++i)
      {
        s.push_back (random_char_of (chars));
      }

      return s;
    }

    std::string random_string()
    {
      std::string chars;

      for (int i = 0; i < 256; ++i)
      {
        chars.push_back (char (i));
      }

      return random_string_of (chars);
    }

    std::string random_string_without (std::string const& except)
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

    std::string random_identifier()
    {
      return
        fhg::util::random_char_of
        ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_")
        +
        fhg::util::random_string_of
        ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
    }

    std::string random_content_string()
    {
      std::string const zero (1, '\0');
      std::string const forbidden (zero + "<>\\");

      return fhg::util::random_string_without (forbidden);
    }

    std::string random_string_without_zero()
    {
      static std::string const zero (1, '\0');

      return fhg::util::random_string_without (zero);
    }
  }
}
