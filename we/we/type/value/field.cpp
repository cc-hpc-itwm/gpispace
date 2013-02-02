// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/field.hpp>

#include <we/type/literal.hpp>
#include <we/type/literal/show.hpp>

#include <stdexcept>
#include <sstream>

namespace value
{
  namespace
  {
    class visitor_field : public boost::static_visitor<type&>
    {
    private:
      signature::field_name_t name;

    public:
      visitor_field (const signature::field_name_t& _name)
        : name (_name)
      {}

      type& operator() (structured_t& s) const
      {
        return s[name];
      }

      type& operator() (literal::type& l) const
      {
        std::ostringstream s;

        s << "cannot get field " << name
          << " from the literal " << literal::show (l)
          ;

        throw std::runtime_error (s.str());
      }
    };
  }

  type& field (const signature::field_name_t& field, type& v)
  {
    return boost::apply_visitor (visitor_field (field), v);
  }
}
