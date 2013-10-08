// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/read_bool.hpp>

#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

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

      parse::position pos (inp);

      return parse::require::boolean (pos);
    }
  }
}
