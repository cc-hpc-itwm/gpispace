// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/parse/position.hpp>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/require.hpp>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      position::position (const std::string& input)
        : _k (0)
        , _pos (input.begin())
        , _begin (input.begin())
        , _end (input.end())
      {}
      position::position ( const std::string::const_iterator& begin
                         , const std::string::const_iterator& end
                         )
        : _k (0)
        , _pos (begin)
        , _begin (begin)
        , _end (end)
      {}

      std::string position::consumed() const
      {
        return std::string (_begin, _pos);
      }
      std::string position::rest() const
      {
        return std::string (_pos, _end);
      }
    }
  }
}
