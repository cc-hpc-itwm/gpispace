// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_READ_BOOL_HPP
#define _FHG_UTIL_READ_BOOL_HPP 1

#include <boost/unordered_set.hpp>

#include <algorithm>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace detail
    {
      struct bool_names
      {
      private:
        typedef boost::unordered_set<std::string> set_type;

        set_type _true;
        set_type _false;

        inline bool elem (const set_type & set, const std::string & s) const
        {
          std::string lowered (s);

          std::transform ( lowered.begin(), lowered.end()
                         , lowered.begin(), tolower
                         );

          return set.find (lowered) != set.end();
        }

      public:
        bool_names (void) : _true(), _false()
        {
          _true.insert ("true");
          _true.insert ("yes");
          _true.insert ("1");

          _false.insert ("false");
          _false.insert ("no");
          _false.insert ("0");
        }

        bool is_true (const std::string & s) const { return elem (_true, s); }
        bool is_false (const std::string & s) const { return elem (_false, s); }
      };
    }

    inline bool
    read_bool (const std::string & inp)
    {
      static detail::bool_names n;

      if (n.is_true (inp))
        {
          return true;
        }
      else if (n.is_false (inp))
        {
          return false;
        }
      else
        {
          throw std::runtime_error ("failed to read a bool from: '" + inp + "'");
        }
    }
  }
}

#endif
