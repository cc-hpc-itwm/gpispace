// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_REQUIRE_TYPE_HPP
#define _WE_TYPE_LITERAL_REQUIRE_TYPE_HPP

#include <we/type/signature.hpp>
#include <we/type/error.hpp>

#include <we/type/literal/control.hpp>
#include <we/type/bitsetofint.hpp>
#include <we/type/bytearray.hpp>
#include <we/type/literal.hpp>
#include <we/type/literal/name.hpp>

namespace literal
{
  namespace visitor
  {
    class type_name : public boost::static_visitor<type_name_t>
    {
    public:
      type_name_t operator () (const we::type::literal::control&) const
      {
        return CONTROL();
      }
      type_name_t operator () (const bool &) const { return BOOL(); }
      type_name_t operator () (const long &) const { return LONG(); }
      type_name_t operator () (const double &) const { return DOUBLE(); }
      type_name_t operator () (const char &) const { return CHAR(); }
      type_name_t operator () (const std::string &) const { return STRING(); }
      type_name_t operator () (const bitsetofint::type &) const { return BITSET(); }
      type_name_t operator () (const literal::stack_type &) const { return STACK(); }
      type_name_t operator () (const literal::map_type &) const { return MAP(); }
      type_name_t operator () (const literal::set_type &) const { return SET(); }
      type_name_t operator () (const bytearray::type &) const { return BYTEARRAY(); }
    };
  }

  inline const type & require_type ( const signature::field_name_t & field
                                   , const type_name_t & req
                                   , const type & x
                                   )
  {
    const type_name_t has (boost::apply_visitor (visitor::type_name(), x));

    if (has != req)
      throw ::type::error (field, req, has);

    return x;
  }
}

#endif
