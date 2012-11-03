// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/parent.hpp>

#include <xml/parse/id/mapper.hpp>

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/token.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/use.hpp>

#define PARENT_IMPL(PARENT,TYPE)                \
  bool TYPE ## _type::has_parent() const        \
  {                                             \
    return _parent;                             \
  }                                             \
  boost::optional<const PARENT ## _type&>       \
  TYPE ## _type::parent() const                 \
  {                                             \
    return id_mapper()->get (_parent);          \
  }                                             \
  boost::optional<PARENT ## _type&>             \
  TYPE ## _type::parent()                       \
  {                                             \
    return id_mapper()->get_ref (_parent);      \
  }

namespace xml
{
  namespace parse
  {
    namespace type
    {
      PARENT_IMPL(transition,connect)
      PARENT_IMPL(function,expression)
      PARENT_IMPL(function,module)
      PARENT_IMPL(function,net)
      PARENT_IMPL(net,place)
      PARENT_IMPL(function,port)
      PARENT_IMPL(net,specialize)
      PARENT_IMPL(function,structure)
      PARENT_IMPL(net,tmpl)
      PARENT_IMPL(place,token)
      PARENT_IMPL(net,transition)
      PARENT_IMPL(transition,use)
    }
  }
}

#undef PARENT_IMPL
