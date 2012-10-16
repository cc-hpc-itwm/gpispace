// bernd.loerwald@itwm.fraunhofer.de

#ifndef XML_UTIL_ID_TYPE_HPP
#define XML_UTIL_ID_TYPE_HPP

#include <boost/cstdint.hpp>

namespace fhg
{
  namespace xml
  {
    namespace parse
    {
      namespace util
      {
        typedef boost::uint_fast64_t id_type;
      }
    }
  }
}
//! \todo The remainder of xml/ is in ::xml::parse, not
//! ::fhg::xml::parse. One should move them all to the same namespace.
namespace xml
{
  namespace parse
  {
    namespace id
    {
      //! \todo Let them be of some class to prevent casting.
#define MAKE_ID(name)                           \
      typedef ::fhg::xml::parse::util::id_type name

      MAKE_ID (connect);
      MAKE_ID (net);
      MAKE_ID (place);
      MAKE_ID (port);
      MAKE_ID (transition);

#undef MAKE_ID
    }
  }
}

#endif
