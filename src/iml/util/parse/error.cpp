#include <iml/util/parse/error.hpp>

namespace fhg
{
  namespace iml
  {
    namespace util
    {
      namespace parse
      {
        namespace error
        {
          expected::expected (const std::string& what, const position& inp)
            : generic (boost::format ("expected '%1%'") % what, inp)
          {}
        }
      }
    }
  }
}
