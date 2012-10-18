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
#define MAKE_ID(name)                                                   \
      struct name                                                       \
      {                                                                 \
      public:                                                           \
        name (const boost::uint_fast64_t& val);                         \
        bool operator< (const name& other) const;                       \
        bool operator== (const name& other) const;                      \
        name& operator= (const boost::uint_fast64_t& val);              \
        friend std::size_t hash_value (const name&);                    \
      private:                                                          \
        boost::uint_fast64_t _val;                                      \
      };                                                                \
      std::size_t hash_value (const name& val)

      MAKE_ID (connect);
      MAKE_ID (expression);
      MAKE_ID (function);
      MAKE_ID (module);
      MAKE_ID (net);
      MAKE_ID (place);
      MAKE_ID (place_map);
      MAKE_ID (port);
      MAKE_ID (specialize);
      MAKE_ID (structure);
      MAKE_ID (tmpl);
      MAKE_ID (token);
      MAKE_ID (transition);
      MAKE_ID (use);

#undef MAKE_ID
    }
  }
}

#endif
