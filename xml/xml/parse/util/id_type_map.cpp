// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/util/id_type_map.hpp>

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
#include <xml/parse/type/token.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/use.hpp>

#include <fhg/util/backtracing_exception.hpp>

#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>

namespace xml
{
  namespace parse
  {
    namespace id_map
    {
      struct mapper::maps
      {
        maps()
          : _dummy_to_use_macro_in_initializer_list (true)
#define ITEM(__IGNORE,MEMBER_NAME,__IGNORE2)    \
                                                \
          , MEMBER_NAME()                       \
          , MEMBER_NAME ## _references()

#include <xml/parse/util/id_type_map_helper.lst>
#undef ITEM
        { }

      private:
        bool _dummy_to_use_macro_in_initializer_list;

      public:

#define ITEM(NAME,MEMBER_NAME,TYPE)                                     \
                                                                        \
        typedef boost::unordered_map< id::base_id_type                  \
                                    , type::TYPE                        \
                                    > NAME ## _map_type;                \
                                                                        \
        NAME ## _map_type MEMBER_NAME;                                  \
        boost::unordered_map<id::base_id_type, id::base_id_type>        \
          MEMBER_NAME ## _references;

#include <xml/parse/util/id_type_map_helper.lst>
#undef ITEM
      };

      mapper::mapper()
        : _maps (new maps)
      { }
      mapper::~mapper()
      { }

#define STRINGIFY(s) #s
#define EXPAND_AND_STRINGIFY(s) STRINGIFY(s)

#define ITEM(NAME,MEMBER_NAME,TYPE)                                     \
                                                                        \
      boost::optional<type::TYPE>                                       \
        mapper::get (const id::NAME& id) const                          \
      {                                                                 \
        const maps::NAME ## _map_type::const_iterator elem              \
          (_maps->MEMBER_NAME.find (id._val));                          \
        return boost::make_optional ( elem != _maps->MEMBER_NAME.end()  \
                                    , elem->second                      \
                                    );                                  \
      }                                                                 \
                                                                        \
      void mapper::put (const id::NAME& id, const type::TYPE& elem)     \
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
        _maps->MEMBER_NAME.insert (std::make_pair (id._val, elem));     \
      }                                                                 \
                                                                        \
      void mapper::add_reference (const id::NAME& id)                   \
      {                                                                 \
        ++_maps->MEMBER_NAME ## _references[id._val];                   \
      }                                                                 \
                                                                        \
      void mapper::remove_reference (const id::NAME& id)                \
      {                                                                 \
        if (!(--_maps->MEMBER_NAME ## _references[id._val]))            \
        {                                                               \
          _maps->MEMBER_NAME.erase (id._val);                           \
          _maps->MEMBER_NAME ## _references.erase (id._val);            \
        }                                                               \
      }

#include <xml/parse/util/id_type_map_helper.lst>
#undef ITEM

#undef STRINIGFY
#undef EXPAND_AND_STRINGIFY

    }
  }
}

