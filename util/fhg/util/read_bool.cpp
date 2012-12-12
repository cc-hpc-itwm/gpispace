// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/read_bool.hpp>

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    bool read_bool (const std::string& _inp)
    {
      std::string inp;

      std::transform ( _inp.begin(), _inp.end()
                     , std::back_inserter (inp), tolower
                     );

      if (inp == "true" || inp == "yes" || inp == "1" || inp == "on")
        {
          return true;
        }

      if (inp == "false" || inp == "no" || inp == "0" || inp == "off")
        {
          return false;
        }

      throw std::runtime_error ("failed to read a bool from: '" + _inp + "'");
    }
  }
}
