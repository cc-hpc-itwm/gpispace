// mirko.rahn@itwm.fhg.de

#include <xml/parse/id/generic.hpp>

#include <xml/parse/id/mapper.hpp>

#include <xml/parse/type/function.hpp>

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


namespace xml
{
  namespace parse
  {
    namespace type
    {
      ID_IMPL(function)
    }
  }
}

#undef ID_IMPL
