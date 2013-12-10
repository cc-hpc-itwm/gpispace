// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/id/types.hpp>

#include <xml/parse/id/mapper.hpp>

#include <iostream>

#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace id
    {
#define ITEM(NAME,__IGNORE,__IGNORE2,__IGNORE3)                         \
      NAME::NAME (const base_id_type& val)                              \
        :  _val (val)                                                   \
      { }                                                               \
                                                                        \
      bool NAME::operator< (const NAME& other) const                    \
      {                                                                 \
        return _val < other._val;                                       \
      }                                                                 \
      bool NAME::operator== (const NAME& other) const                   \
      {                                                                 \
        return _val == other._val;                                      \
      }                                                                 \
      bool NAME::operator!= (const NAME& other) const                   \
      {                                                                 \
        return _val != other._val;                                      \
      }                                                                 \
                                                                        \
      std::size_t hash_value (const NAME& val)                          \
      {                                                                 \
        return boost::hash<base_id_type>() (val._val);                  \
      }                                                                 \
      std::ostream& operator<< (std::ostream& os, const NAME& val)      \
      {                                                                 \
        return os << val._val;                                          \
      }                                                                 \
                                                                        \
      bool operator< (const NAME& lhs, const ref::NAME& rhs)            \
      {                                                                 \
        return lhs < rhs.id();                                          \
      }                                                                 \
      bool operator< (const ref::NAME& lhs, const NAME& rhs)            \
      {                                                                 \
        return lhs.id() < rhs;                                          \
      }                                                                 \
      bool operator== (const NAME& lhs, const ref::NAME& rhs)           \
      {                                                                 \
        return lhs == rhs.id();                                         \
      }                                                                 \
      bool operator== (const ref::NAME& lhs, const NAME& rhs)           \
      {                                                                 \
        return lhs.id() == rhs;                                         \
      }                                                                 \
      bool operator!= (const NAME& lhs, const ref::NAME& rhs)           \
      {                                                                 \
        return lhs != rhs.id();                                         \
      }                                                                 \
      bool operator!= (const ref::NAME& lhs, const NAME& rhs)           \
      {                                                                 \
        return lhs.id() != rhs;                                         \
      }


#include <xml/parse/id/helper.lst>

#undef ITEM

      namespace ref
      {
#define ITEM(NAME,__IGNORE,XML_TYPE,__IGNORE3)                          \
        NAME::NAME (const id::NAME& id, mapper* mapper_)                \
          :  _id (id)                                                   \
          , _mapper (mapper_)                                           \
        {                                                               \
          _mapper->add_reference (*this);                               \
        }                                                               \
        NAME::NAME (const NAME& other)                                  \
          :  _id (other._id)                                            \
          , _mapper (other._mapper)                                     \
        {                                                               \
          _mapper->add_reference (*this);                               \
        }                                                               \
        NAME& NAME::operator= (const NAME& other)                       \
        {                                                               \
          _mapper->remove_reference (*this);                            \
          _id = other._id;                                              \
          _mapper = other._mapper;                                      \
          _mapper->add_reference (*this);                               \
          return *this;                                                 \
        }                                                               \
        NAME::~NAME()                                                   \
        {                                                               \
          _mapper->remove_reference (*this);                            \
          _mapper = NULL;                                               \
        }                                                               \
                                                                        \
        bool NAME::operator< (const NAME& other) const                  \
        {                                                               \
          return _id < other._id;                                       \
        }                                                               \
        bool NAME::operator== (const NAME& other) const                 \
        {                                                               \
          return _id == other._id;                                      \
        }                                                               \
        bool NAME::operator!= (const NAME& other) const                 \
        {                                                               \
          return _id != other._id;                                      \
        }                                                               \
                                                                        \
        const type::XML_TYPE& NAME::get() const                         \
        {                                                               \
          return *_mapper->get (*this);                                 \
        }                                                               \
        type::XML_TYPE& NAME::get_ref() const                           \
        {                                                               \
          return *_mapper->get_ref (*this);                             \
        }                                                               \
                                                                        \
        const id::NAME& NAME::id() const                                \
        {                                                               \
          return _id;                                                   \
        }                                                               \
        mapper* NAME::id_mapper() const                                 \
        {                                                               \
          return _mapper;                                               \
        }                                                               \
                                                                        \
        std::size_t hash_value (const NAME& ref)                        \
        {                                                               \
          return hash_value (ref._id);                                  \
        }                                                               \
        std::ostream& operator<< (std::ostream& os, const NAME& ref)    \
        {                                                               \
          return os << ref._id;                                         \
        }

#include <xml/parse/id/helper.lst>

#undef ITEM
      }
    }
  }
}
