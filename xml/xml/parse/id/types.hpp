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

#define ITEM(NAME,__IGNORE,__IGNORE2,__IGNORE3)                         \
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
      std::ostream& operator<< (std::ostream& os, const NAME& val);

#include <xml/parse/id/helper.lst>

#undef ITEM
    }
  }
}

#endif
