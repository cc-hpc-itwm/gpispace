// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/parse/error.hpp>

#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      namespace error
      {
        namespace detail
        {
          std::string nice (const std::string& msg, const position& inp)
          {
            std::ostringstream oss;

            oss << "PARSE ERROR [" << inp() << "]: " << msg << std::endl;
            oss << inp.consumed() << " " << inp.rest() << std::endl;

            for (std::size_t i (0); i < inp(); ++i)
            {
              oss << " ";
            }
            oss << "^" << std::endl;

            return oss.str();
          }
        }

        expected::expected (const std::string& what, const position& inp)
          : generic (boost::format ("expected '%1%'") % what, inp)
        {}

        signed_unsigned::signed_unsigned (const position& pos)
          : generic ("unsigned value but sign found", pos)
        {}
      }
    }
  }
}
