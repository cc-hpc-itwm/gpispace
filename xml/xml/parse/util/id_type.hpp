// bernd.loerwald@itwm.fraunhofer.de

#ifndef XML_UTIL_ID_TYPE_HPP
#define XML_UTIL_ID_TYPE_HPP

#include <iosfwd>

#include <boost/cstdint.hpp>

namespace xml
{
  namespace parse
  {
    namespace id
    {
      typedef boost::uint_fast64_t base_id_type;

#define MAKE_ID(name)                                                   \
      struct name                                                       \
      {                                                                 \
      public:                                                           \
        name (const base_id_type& val);                                 \
        bool operator< (const name& other) const;                       \
        bool operator== (const name& other) const;                      \
        name& operator= (const base_id_type& val);                      \
        friend std::size_t hash_value (const name&);                    \
        friend std::ostream& operator<< (std::ostream&, const name&);   \
      private:                                                          \
        base_id_type _val;                                              \
      };                                                                \
      std::size_t hash_value (const name& val);                         \
      std::ostream& operator<< (std::ostream& os, const name& val)

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
