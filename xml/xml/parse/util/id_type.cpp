// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/util/id_type.hpp>

#include <iostream>

#include <boost/functional/hash.hpp>

namespace xml
{
  namespace parse
  {
    namespace id
    {
#define MAKE_ID(name)                                                   \
      name::name (const base_id_type& val)                              \
        :  _val (val)                                                   \
      { }                                                               \
      bool name::operator< (const name& other) const                    \
      {                                                                 \
        return _val < other._val;                                       \
      }                                                                 \
      bool name::operator== (const name& other) const                   \
      {                                                                 \
        return _val == other._val;                                      \
      }                                                                 \
      name& name::operator= (const base_id_type& val)                   \
      {                                                                 \
        _val = val;                                                     \
        return *this;                                                   \
      }                                                                 \
      std::size_t hash_value (const name& val)                          \
      {                                                                 \
        return boost::hash< base_id_type>()                             \
          (val._val);                                                   \
      }                                                                 \
      std::ostream& operator<< (std::ostream& os, const name& val)      \
      {                                                                 \
        return os << val._val;                                          \
      }

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

