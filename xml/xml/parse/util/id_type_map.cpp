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

#include <map>

#include <boost/format.hpp>
#include <boost/optional.hpp>

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
          , MEMBER_NAME()
#include <xml/parse/util/id_type_map_helper.lst>
#undef ITEM
        { }

      private:
        bool _dummy_to_use_macro_in_initializer_list;

      public:
#define ITEM(TYPE,NAME,__IGNORE)                \
        TYPE NAME;
#include <xml/parse/util/id_type_map_helper.lst>
#undef ITEM
      };

      mapper::mapper()
        : _maps (new maps)
      { }

      mapper::~mapper()
      { }

#define ITEM(NAME,MEMBER_NAME,TYPE)                                     \
      boost::optional<type::TYPE> mapper::get (id::NAME id) const       \
      {                                                                 \
        const NAME::const_iterator elem (_maps->MEMBER_NAME.find (id)); \
        return boost::make_optional ( elem != _maps->MEMBER_NAME.end()  \
                                    , elem->second                      \
                                    );                                  \
      }
#include <xml/parse/util/id_type_map_helper.lst>
#undef ITEM

#define STRINGIFY(s) #s
#define EXPAND_AND_STRINGIFY(s) STRINGIFY(s)

#define ITEM(NAME,MEMBER_NAME,TYPE)                                     \
      void mapper::put (id::NAME id, type::TYPE elem)                   \
      {                                                                 \
        if (_maps->MEMBER_NAME.find (id) != _maps->MEMBER_NAME.end())   \
        {                                                               \
          throw fhg::util::backtracing_exception                        \
            ( ( boost::format ( "Adding %1% failed: Element "           \
                              "with ID %2% already exists."             \
                              )                                         \
              % EXPAND_AND_STRINGIFY (NAME)                             \
              % id                                                      \
              ).str()                                                   \
            );                                                          \
        }                                                               \
        _maps->MEMBER_NAME.insert (std::make_pair (id, elem));          \
      }
#include <xml/parse/util/id_type_map_helper.lst>
#undef ITEM
    }
  }
}
