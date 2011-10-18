// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_MK_STRUCTURED_HPP
#define _WE_TYPE_VALUE_MK_STRUCTURED_HPP

#include <we/type/value.hpp>

#include <we/type/literal.hpp>

namespace value
{
  namespace visitor
  {
    class mk_structured : public boost::static_visitor<type&>
    {
    private:
      type& _x;

    public:
      mk_structured (type& x) : _x (x) {}

      type& operator () (const literal::type &) const
      {
        return _x = structured_t();
      }

      template<typename T> type& operator () (const T&) const { return _x; }
    };

    inline type& mk_structured_or_keep (type& x)
    {
      return boost::apply_visitor (value::visitor::mk_structured (x), x);
    }
  }
}

#endif
