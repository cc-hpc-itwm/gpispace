// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/cpp/include.hpp>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      include::include (const std::string& fname)
        : _fname (fname)
      {}
      include::include (const boost::filesystem::path& fname)
        : _fname (fname.string())
      {}
      std::ostream& include::operator() (std::ostream& os) const
      {
        if (_fname.size())
        {
          os << "#include <" << _fname << ">" << std::endl;
        }

        return os;
      }
    }
  }
}
