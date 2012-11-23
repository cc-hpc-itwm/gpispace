// bernd.loerwald@itwm.fraunhofer.de

#ifndef XML_PARSE_ID_TYPES_HPP
#define XML_PARSE_ID_TYPES_HPP

#include <xml/parse/id/types.fwd.hpp>

#include <xml/parse/id/mapper.fwd.hpp>
#include <xml/parse/type/connect.fwd.hpp>
#include <xml/parse/type/expression.fwd.hpp>
#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/mod.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/place.fwd.hpp>
#include <xml/parse/type/place_map.fwd.hpp>
#include <xml/parse/type/port.fwd.hpp>
#include <xml/parse/type/specialize.fwd.hpp>
#include <xml/parse/type/struct.fwd.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/type/use.fwd.hpp>

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
        NAME (const base_id_type& val);                                 \
                                                                        \
        bool operator< (const NAME& other) const;                       \
        bool operator== (const NAME& other) const;                      \
        bool operator!= (const NAME& other) const;                      \
                                                                        \
        friend std::size_t hash_value (const NAME&);                    \
        friend std::ostream& operator<< (std::ostream&, const NAME&);   \
        friend struct ref::NAME;                                        \
        friend class ::xml::parse::id::mapper;                          \
                                                                        \
      private:                                                          \
        base_id_type _val;                                              \
      };                                                                \
                                                                        \
      std::size_t hash_value (const NAME& val);                         \
      std::ostream& operator<< (std::ostream& os, const NAME& val);     \
                                                                        \
      bool operator< (const NAME& lhs, const ref::NAME& rhs);           \
      bool operator< (const ref::NAME& lhs, const NAME& rhs);           \
      bool operator== (const NAME& lhs, const ref::NAME& rhs);          \
      bool operator== (const ref::NAME& lhs, const NAME& rhs);          \
      bool operator!= (const NAME& lhs, const ref::NAME& rhs);          \
      bool operator!= (const ref::NAME& lhs, const NAME& rhs);

#include <xml/parse/id/helper.lst>

#undef ITEM

      namespace ref
      {
#define ITEM(NAME,__IGNORE,XML_TYPE,__IGNORE2)                          \
        struct NAME                                                     \
        {                                                               \
        public:                                                         \
          NAME (const id::NAME& id, mapper* mapper_);                   \
          NAME (const NAME& other);                                     \
          NAME& operator= (const NAME& other);                          \
          ~NAME();                                                      \
                                                                        \
          bool operator< (const NAME& other) const;                     \
          bool operator== (const NAME& other) const;                    \
          bool operator!= (const NAME& other) const;                    \
                                                                        \
          const type::XML_TYPE& get() const;                            \
          type::XML_TYPE& get_ref() const;                              \
                                                                        \
          const id::NAME& id() const;                                   \
          mapper* id_mapper() const;                                    \
                                                                        \
          friend std::size_t hash_value (const NAME&);                  \
          friend std::ostream& operator<< (std::ostream&, const NAME&); \
          friend class ::xml::parse::id::mapper;                        \
                                                                        \
        private:                                                        \
          id::NAME _id;                                                 \
          mapper* _mapper;                                              \
        };                                                              \
                                                                        \
        std::size_t hash_value (const NAME& val);                       \
        std::ostream& operator<< (std::ostream& os, const NAME& val);

#include <xml/parse/id/helper.lst>

#undef ITEM
      }
    }
  }
}

#endif
