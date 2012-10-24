// bernd.loerwald@itwm.fraunhofer.de

#ifndef XML_PARSE_ID_TYPES_HPP
#define XML_PARSE_ID_TYPES_HPP

#include <xml/parse/id/types.fwd.hpp>

#include <xml/parse/id/mapper.fwd.hpp>

#include <iosfwd>

#include <boost/cstdint.hpp>

namespace xml
{
  namespace parse
  {
    namespace id
    {
      typedef boost::uint_fast64_t base_id_type;

#define MAKE_ID(NAME)                                                   \
      struct NAME                                                       \
      {                                                                 \
      public:                                                           \
        NAME (const base_id_type& val, mapper* mapper_);                \
        NAME (const NAME& other);                                       \
        NAME& operator= (const NAME& other);                            \
        ~NAME();                                                        \
                                                                        \
        bool operator< (const NAME& other) const;                       \
        bool operator== (const NAME& other) const;                      \
                                                                        \
        friend std::size_t hash_value (const NAME&);                    \
        friend std::ostream& operator<< (std::ostream&, const NAME&);   \
        friend class mapper;                                            \
                                                                        \
      private:                                                          \
        base_id_type _val;                                              \
        mapper* _mapper;                                                \
      };                                                                \
                                                                        \
      std::size_t hash_value (const NAME& val);                         \
      std::ostream& operator<< (std::ostream& os, const NAME& val)

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
