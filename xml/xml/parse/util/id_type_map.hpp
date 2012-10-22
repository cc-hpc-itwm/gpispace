// bernd.loerwald@itwm.fraunhofer.de

#ifndef XML_UTIL_ID_TYPE_MAP_HPP
#define XML_UTIL_ID_TYPE_MAP_HPP

#include <xml/parse/util/id_type_map.fwd.hpp>

#include <xml/parse/util/id_type.fwd.hpp>

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
#include <xml/parse/type/token.fwd.hpp>
#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/type/use.fwd.hpp>

#include <map>

#include <boost/optional/optional_fwd.hpp>

namespace xml
{
  namespace parse
  {
    namespace id_map
    {
      class mapper
      {
      private:
#define ITEM(NAME,__IGNORE,TYPE)                    \
        typedef std::map<id::NAME,type::TYPE> NAME;
#include <xml/parse/util/id_type_map_helper.lst>
#undef ITEM

      public:
        mapper();

#define ITEM(NAME,__IGNORE,TYPE)                              \
        boost::optional<type::TYPE> get (id::NAME id) const;
#include <xml/parse/util/id_type_map_helper.lst>
#undef ITEM

#define ITEM(NAME,__IGNORE,TYPE)                        \
        void put (id::NAME id, type::TYPE elem);
#include <xml/parse/util/id_type_map_helper.lst>
#undef ITEM

      private:
        bool _dummy_to_use_macro_in_initializer_list;

#define ITEM(TYPE,NAME,__IGNORE)                \
        TYPE NAME;
#include <xml/parse/util/id_type_map_helper.lst>
#undef ITEM
      };
    }
  }
}

#endif
