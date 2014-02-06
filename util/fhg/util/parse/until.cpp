// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/parse/until.hpp>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      std::string until
        (position& pos, boost::function<bool (position const&)> const& p)
      {
        std::string s;

        while (!pos.end() && !p (pos))
        {
          s += *pos;

          ++pos;
        }

        return s;
      }
    }
  }
}
