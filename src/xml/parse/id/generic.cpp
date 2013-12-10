// mirko.rahn@itwm.fhg.de

#include <xml/parse/id/generic.hpp>

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

#define ID_IMPL(TYPE)                                       \
  const id::TYPE& TYPE ## _type::id() const                 \
  {                                                         \
    return _id;                                             \
  }                                                         \
  id::mapper* TYPE ## _type::id_mapper() const              \
  {                                                         \
    return _id_mapper;                                      \
  }                                                         \
  id::ref::TYPE TYPE ## _type::make_reference_id() const    \
  {                                                         \
    return id::ref::TYPE (id(), id_mapper());               \
  }


#define PARENT_IMPL(PARENT,TYPE)                                        \
  bool TYPE ## _type::has_parent() const                                \
  {                                                                     \
    return _parent;                                                     \
  }                                                                     \
  boost::optional<const PARENT ## _type&>                               \
  TYPE ## _type::parent() const                                         \
  {                                                                     \
    return id_mapper()->get (_parent);                                  \
  }                                                                     \
  boost::optional<PARENT ## _type&>                                     \
  TYPE ## _type::parent()                                               \
  {                                                                     \
    return id_mapper()->get_ref (_parent);                              \
  }                                                                     \
  void TYPE ## _type::unparent()                                        \
  {                                                                     \
    _parent = boost::none;                                              \
  }                                                                     \
  void TYPE ## _type::parent (const id::PARENT& parent)                 \
  {                                                                     \
    _parent = boost::make_optional (parent);                            \
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
      ID_IMPL(net)
      ID_IMPL(place)
      ID_IMPL(place_map)
      ID_IMPL(port)
      ID_IMPL(specialize)
      ID_IMPL(structure)
      ID_IMPL(tmpl)
      ID_IMPL(transition)
      ID_IMPL(use)

      PARENT_IMPL(function,expression)
      PARENT_IMPL(function,module)
      PARENT_IMPL(function,net)
      PARENT_IMPL(function,port)
      PARENT_IMPL(function,structure)
      PARENT_IMPL(net,place)
      PARENT_IMPL(net,specialize)
      PARENT_IMPL(net,tmpl)
      PARENT_IMPL(net,transition)
      PARENT_IMPL(transition,connect)
      PARENT_IMPL(transition,place_map)
      PARENT_IMPL(transition,use)
    }
  }
}

#undef ID_IMPL
#undef PARENT_IMPL
