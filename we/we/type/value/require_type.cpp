// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/require_type.hpp>

#include <we/type/literal.hpp>
#include <we/type/literal/show.hpp>
#include <we/type/literal/name.hpp>
#include <we/type/literal/require_type.hpp>

#include <we/type/error.hpp>

#include <sstream>

namespace value
{
  namespace
  {
    class visitor_require_type : public boost::static_visitor<type>
    {
    private:
      const signature::field_name_t & field_name;

    public:
      visitor_require_type (const signature::field_name_t & _field_name)
        : field_name (_field_name)
      {}

      type operator () ( const literal::type_name_t & type_name
                       , const literal::type & v
                       ) const
      {
        return literal::require_type (field_name, type_name, v);
      }

      type operator () ( const signature::structured_t & signature
                       , const structured_t & v
                       ) const
      {
        for ( signature::structured_t::const_iterator sig (signature.begin())
            ; sig != signature.end()
            ; ++sig
            )
          {
            const structured_t::const_iterator pos (v.find (sig->first));

            if (!v.has_field (sig->first))
              {
                throw ::type::error
                  ( "require_field: missing (or uninitialized) field "
                  + sig->first
                  );
              }

            value::require_type ( field_name + "." + sig->first
                                , sig->second
                                , pos->second
                                );
          }

        for ( structured_t::const_iterator field (v.begin())
            ; field != v.end()
            ; ++field
            )
          if (!signature.has_field (field->first))
            throw ::type::error ("unknown field " + field->first);

        return v;
      }

      template<typename T>
      type operator () (const T & t, const literal::type & u) const
      {
        std::ostringstream s;

        s << "incompatible types:"
          << " wanted type " << fhg::util::show (t)
          << " given value " << literal::show (u)
          ;

        throw ::type::error (s.str());
      }

      template<typename U>
      type operator () (const literal::type & t, const U & u) const
      {
        std::ostringstream s;

        s << "incompatible types:"
          << " wanted type " << literal::show (t)
          << " given value " << fhg::util::show (u)
          ;

        throw ::type::error (s.str());
      }

      template<typename T, typename U>
      type operator () (const T & t, const U & u) const
      {
        std::ostringstream s;

        s << "incompatible types:"
          << " wanted type " << fhg::util::show (t)
          << " given value " << fhg::util::show (u)
          ;

        throw ::type::error (s.str());
      }
    };
  }

  type require_type ( const signature::field_name_t& field
                    , const signature::type& sig
                    , const value::type& v
                    )
  {
    return boost::apply_visitor (visitor_require_type (field), sig.desc(), v);
  }
}
