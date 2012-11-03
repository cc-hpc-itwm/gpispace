// mirko.rahn@itwm.fhg.de

#include <xml/parse/id/generic.hpp>

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/token.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/use.hpp>
#include <xml/parse/type/transition.hpp>

#define ID_IMPL(TYPE)                                   \
  const id::TYPE& TYPE ## _type::id() const             \
  {                                                     \
    return _id;                                         \
  }                                                     \
  id::mapper* TYPE ## _type::id_mapper() const          \
  {                                                     \
    return _id_mapper;                                  \
  }

namespace xml
{
  namespace parse
  {
    namespace type
    {
      ID_IMPL(connect)
      ID_IMPL(expression)
      ID_IMPL(function)
      ID_IMPL(module)
      ID_IMPL(place)
      ID_IMPL(place_map)
      ID_IMPL(port)
      ID_IMPL(specialize)
      ID_IMPL(structure)
      ID_IMPL(tmpl)
      ID_IMPL(token)
      ID_IMPL(net)
      ID_IMPL(use)
      ID_IMPL(transition)
    }
  }
}

#undef ID_IMPL
