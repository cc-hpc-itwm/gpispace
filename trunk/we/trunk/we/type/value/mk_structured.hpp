// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_MK_STRUCTURED_HPP
#define _WE_TYPE_VALUE_MK_STRUCTURED_HPP

#include <we/type/value.hpp>

#include <we/type/literal.hpp>

namespace value
{
  namespace visitor
  {
    class mk_structured : public boost::static_visitor<type>
    {
    public:
      type operator () (const literal::type &) const { return structured_t(); }
      type operator () (const structured_t & s) const { return s; }
    };
  }
}

#endif
