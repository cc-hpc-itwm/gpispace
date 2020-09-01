#include <iml/util/read_bool.hpp>

#include <iml/util/parse/position.hpp>
#include <iml/util/parse/require.hpp>

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace fhg
{
  namespace iml
{
  namespace util
  {
    bool read_bool (const std::string& _inp)
    {
      std::string inp;

      std::transform ( _inp.begin(), _inp.end()
                     , std::back_inserter (inp), tolower
                     );

      parse::position_string pos (inp);

      return parse::require::boolean (pos);
    }
  }
}
}
