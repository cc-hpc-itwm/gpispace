// mirko.rahn@itwm.fraunhofer.de

#include <we/type/literal/require_type.hpp>

#include <we/type/error.hpp>

#include <we/type/literal/control.hpp>
#include <we/type/bitsetofint.hpp>
#include <we/type/bytearray.hpp>

namespace literal
{
  namespace
  {
    class visitor_type_name : public boost::static_visitor<const type_name_t&>
    {
    public:
#define CASE const type_name_t& operator()

      CASE (const we::type::literal::control&) const { return CONTROL(); }
      CASE (const bool&) const { return BOOL(); }
      CASE (const long&) const { return LONG(); }
      CASE (const double&) const { return DOUBLE(); }
      CASE (const char&) const { return CHAR(); }
      CASE (const std::string&) const { return STRING(); }
      CASE (const bitsetofint::type&) const { return BITSET(); }
      CASE (const literal::stack_type&) const { return STACK(); }
      CASE (const literal::map_type&) const { return MAP(); }
      CASE (const literal::set_type&) const { return SET(); }
      CASE (const bytearray::type&) const { return BYTEARRAY(); }

#undef CASE
    };
  }

  const type& require_type ( const signature::field_name_t& field
                           , const type_name_t& req
                           , const type& x
                           )
  {
    const type_name_t& has (boost::apply_visitor (visitor_type_name(), x));

    if (has != req)
    {
      throw ::type::error (field, req, has);
    }

    return x;
  }
}
