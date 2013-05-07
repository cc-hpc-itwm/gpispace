// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/field.hpp>

#include <we/type/literal.hpp>
#include <we/type/literal/show.hpp>

#include <stdexcept>

namespace value
{
  namespace
  {
    class visitor_field : public boost::static_visitor<type&>
    {
    private:
      std::string name;

    public:
      visitor_field (const std::string& _name)
        : name (_name)
      {}

      type& operator() (structured_t& s) const
      {
        return s[name];
      }

      type& operator() (literal::type& l) const
      {
        throw std::runtime_error ( "cannot get field " + name
                                 + " from the literal " + literal::show (l)
                                 );
      }
    };
  }

  type& field (const std::string& field, type& v)
  {
    return boost::apply_visitor (visitor_field (field), v);
  }
}
