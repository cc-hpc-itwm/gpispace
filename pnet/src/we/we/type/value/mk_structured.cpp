// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/mk_structured.hpp>

#include <we/type/literal.hpp>

namespace value
{
  namespace
  {
    class visitor_mk_structured : public boost::static_visitor<type&>
    {
    private:
      type& _x;

    public:
      visitor_mk_structured (type& x) : _x (x) {}

      type& operator() (const literal::type &) const
      {
        return _x = structured_t();
      }

      template<typename T>
      type& operator() (const T&) const
      {
        return _x;
      }
    };
  }

  type& mk_structured_or_keep (type& x)
  {
    return boost::apply_visitor (visitor_mk_structured (x), x);
  }
}
