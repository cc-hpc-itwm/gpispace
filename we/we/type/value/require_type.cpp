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
    class visitor_require_type : public boost::static_visitor<const type&>
    {
    private:
      const signature::field_name_t & field_name;
      const type& _value;

    public:
      visitor_require_type ( const signature::field_name_t & _field_name
                           , const type& value
                           )
        : field_name (_field_name)
        , _value (value)
      {}

      const type& operator () ( const literal::type_name_t& type_name
                              , const literal::type& v
                              ) const
      {
        literal::require_type (field_name, type_name, v);

        return _value;
      }

      const type& operator () ( const signature::structured_t & signature
                              , const structured_t& v
                              ) const
      {
        for ( signature::structured_t::const_iterator sig (signature.begin())
            ; sig != signature.end()
            ; ++sig
            )
          {
            const map_type::const_iterator pos (v.map().find (sig->first));

            if (pos == v.map().end())
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

        for ( map_type::const_iterator field (v.map().begin())
            ; field != v.map().end()
            ; ++field
            )
          if (!signature.has_field (field->first))
            throw ::type::error ("unknown field " + field->first);

        return _value;
      }

      const type& operator () ( const signature::structured_t& signature
                              , const literal::type& u
                              ) const
      {
        std::ostringstream s;

        s << "incompatible types:"
          << " wanted a structured value of type " << fhg::util::show (signature)
          << " but got the literal value" << literal::show (u)
          ;

        throw ::type::error (s.str());
      }

      const type& operator () ( const literal::type_name_t& type_name
                              , const structured_t& v
                              ) const
      {
        std::ostringstream s;

        s << "incompatible types:"
          << " wanted a literal value of type " << fhg::util::show (type_name)
          << " but got the structured value " << fhg::util::show (v)
          ;

        throw ::type::error (s.str());
      }
    };
  }

  const type& require_type ( const signature::field_name_t& field
                           , const signature::type& sig
                           , const value::type& v
                           )
  {
    return boost::apply_visitor ( visitor_require_type (field, v)
                                , sig.desc()
                                , v
                                );
  }
}
