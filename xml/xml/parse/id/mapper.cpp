// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/id/mapper.hpp>

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/use.hpp>
#include <xml/parse/type/link.hpp>

#include <fhg/util/backtracing_exception.hpp>

#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>
#include <boost/preprocessor.hpp>

namespace xml
{
  namespace parse
  {
    namespace id
    {
      struct mapper::maps
      {
        maps()
#define COLON() :
#define ITEM(__IGNORE,MEMBER_NAME,__IGNORE2,NUMBER)             \
                                                                \
          BOOST_PP_IF ( BOOST_PP_EQUAL (NUMBER, 0)              \
                      , COLON                                   \
                      , BOOST_PP_COMMA                          \
                      )() MEMBER_NAME()                         \
          , MEMBER_NAME ## _references()

#include <xml/parse/id/helper.lst>
#undef ITEM
#undef COLON
        { }

      private:
        bool _dummy_to_use_macro_in_initializer_list;

      public:

#define ITEM(NAME,MEMBER_NAME,TYPE,__IGNORE)                            \
                                                                        \
        typedef boost::unordered_map< NAME                              \
                                    , type::TYPE                        \
                                    > NAME ## _map_type;                \
                                                                        \
        NAME ## _map_type MEMBER_NAME;                                  \
        boost::unordered_map<NAME, base_id_type>                        \
          MEMBER_NAME ## _references;

#include <xml/parse/id/helper.lst>
#undef ITEM
      };

      mapper::mapper()
        : _maps (new maps)
        , _counter()
      { }
      mapper::~mapper()
      { }

#define BODY(NAME,MEMBER_NAME,IT,VALUE)                                 \
                                                                        \
      const maps::NAME ## _map_type::IT elem                            \
        (_maps->MEMBER_NAME.find (VALUE));                              \
                                                                        \
        if (elem == _maps->MEMBER_NAME.end())                           \
          {                                                             \
            return boost::none;                                         \
          }                                                             \
                                                                        \
        return elem->second;                                            \

#define BODY_OPTIONAL(NAME,MEMBER_NAME,IT)                              \
                                                                        \
      if (id)                                                           \
        {                                                               \
          BODY(NAME,MEMBER_NAME,IT,id->_val);                           \
        }                                                               \
                                                                        \
      return boost::none;

#define STRINGIFY(s) #s
#define EXPAND_AND_STRINGIFY(s) STRINGIFY(s)

#define ITEM(NAME,MEMBER_NAME,TYPE,__IGNORE)                            \
                                                                        \
      boost::optional<const type::TYPE&>                                \
        mapper::get (const boost::optional<NAME>& id) const             \
      {                                                                 \
        BODY_OPTIONAL(NAME,MEMBER_NAME,const_iterator);                 \
      }                                                                 \
                                                                        \
      boost::optional<type::TYPE&>                                      \
        mapper::get_ref (const boost::optional<NAME>& id) const         \
      {                                                                 \
        BODY_OPTIONAL(NAME,MEMBER_NAME,iterator);                       \
      }                                                                 \
                                                                        \
      boost::optional<const type::TYPE&>                                \
        mapper::get (const NAME& id) const                              \
      {                                                                 \
        BODY(NAME,MEMBER_NAME,const_iterator,id._val);                  \
      }                                                                 \
                                                                        \
      boost::optional<type::TYPE&>                                      \
        mapper::get_ref (const NAME& id) const                          \
      {                                                                 \
        BODY(NAME,MEMBER_NAME,iterator,id._val);                        \
      }                                                                 \
                                                                        \
      boost::optional<const type::TYPE&>                                \
        mapper::get (const ref::NAME& ref) const                        \
      {                                                                 \
        return get (ref.id());                                          \
      }                                                                 \
                                                                        \
      boost::optional<type::TYPE&>                                      \
        mapper::get_ref (const ref::NAME& ref) const                    \
      {                                                                 \
        return get_ref (ref.id());                                      \
      }                                                                 \
                                                                        \
      void mapper::put (const NAME& id, const type::TYPE& elem)         \
      {                                                                 \
        if ( _maps->MEMBER_NAME.find (id._val)                          \
           != _maps->MEMBER_NAME.end()                                  \
           )                                                            \
        {                                                               \
          throw fhg::util::backtracing_exception                        \
            ( ( boost::format ( "Adding " EXPAND_AND_STRINGIFY (NAME)   \
                          "failed: Element with ID %1% already exists." \
                              )                                         \
              % id                                                      \
              ).str()                                                   \
            );                                                          \
        }                                                               \
        _maps->MEMBER_NAME.insert (std::make_pair (id, elem));          \
      }                                                                 \
                                                                        \
      void mapper::add_reference (const ref::NAME& ref)                 \
      {                                                                 \
        ++_maps->MEMBER_NAME ## _references[ref._id];                   \
      }                                                                 \
                                                                        \
      void mapper::remove_reference (const ref::NAME& ref)              \
      {                                                                 \
        if (!(--_maps->MEMBER_NAME ## _references[ref._id]))            \
        {                                                               \
          _maps->MEMBER_NAME.erase (ref._id);                           \
          _maps->MEMBER_NAME ## _references.erase (ref._id);            \
        }                                                               \
      }

#include <xml/parse/id/helper.lst>
#undef ITEM

#undef BODY
#undef BODY_OPTIONAL

#undef STRINIGFY
#undef EXPAND_AND_STRINGIFY

      id::base_id_type mapper::next_id()
      {
        return _counter.next();
      }
    }
  }
}
