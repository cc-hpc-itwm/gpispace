// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/id/types.hpp>

#include <xml/parse/id/mapper.hpp>

#include <iostream>

#include <boost/functional/hash.hpp>

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
                                                                        \
      std::size_t hash_value (const NAME& val)                          \
      {                                                                 \
        return boost::hash<base_id_type>() (val._val);                  \
      }                                                                 \
      std::ostream& operator<< (std::ostream& os, const NAME& val)      \
      {                                                                 \
        return os << val._val;                                          \
      }

#include <xml/parse/id/helper.lst>

#undef ITEM

      namespace ref
      {
#define ITEM(NAME,__IGNORE,__IGNORE2,__IGNORE3)                         \
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

