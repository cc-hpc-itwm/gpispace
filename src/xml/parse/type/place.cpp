// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/place.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/type/net.hpp>

#include <we/type/signature/is_literal.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      place_type::place_type ( const util::position_type& pod
                             , const std::string & name
                             , const std::string & type
                             , const boost::optional<bool> is_virtual
                             , boost::optional<bool> put_token
                             , std::list<token_type> tokens_
                             , we::type::property::type properties
                             )
        : with_position_of_definition (pod)
        , _is_virtual (is_virtual)
        , _put_token (put_token)
        , _name (name)
        , _type (type)
        , tokens (std::move (tokens_))
        , _properties (std::move (properties))
      {}

      const std::string& place_type::name() const
      {
        return _name;
      }

      const std::string& place_type::type() const
      {
        return _type;
      }

      place_type place_type::with_name (std::string const& new_name) const
      {
        return place_type ( position_of_definition()
                          , new_name
                          , _type
                          , _is_virtual
                          , _put_token
                          , tokens
                          , _properties
                          );
      }

      boost::optional<pnet::type::signature::signature_type>
        place_type::signature (net_type const& parent) const
      {
        if (pnet::type::signature::is_literal (type()))
        {
          return pnet::type::signature::signature_type (type());
        }

        return parent.signature (type());
      }
      pnet::type::signature::signature_type place_type::signature_or_throw
        (net_type const& parent) const
      {
        const boost::optional<pnet::type::signature::signature_type> s
          (signature (parent));

        if (not s)
        {
          throw error::place_type_unknown (*this);
        }

        return *s;
      }

      void place_type::push_token (const token_type & t)
      {
        tokens.push_back (t);
      }

      place_type place_type::specialized ( const type::type_map_type & map_in
                                         , const state::type &
                                         ) const
      {
        const type::type_map_type::const_iterator
          mapped (map_in.find (type()));

        return place_type ( position_of_definition()
                          , _name
                          , mapped != map_in.end() ? mapped->second : type()
                          , _is_virtual
                          , _put_token
                          , tokens
                          , _properties
                          );
      }

      const boost::optional<bool>& place_type::get_is_virtual (void) const
      {
        return _is_virtual;
      }
      bool place_type::is_virtual (void) const
      {
        return _is_virtual.get_value_or (false);
      }

      const we::type::property::type& place_type::properties() const
      {
        return _properties;
      }
      we::type::property::type& place_type::properties()
      {
        return _properties;
      }

      const place_type::unique_key_type& place_type::unique_key() const
      {
        return name();
      }

      namespace dump
      {
        void dump (fhg::util::xml::xmlstream & s, const place_type & p)
        {
          s.open ("place");
          s.attr ("name", p.name());
          s.attr ("type", p.type());
          s.attr ("virtual", p.get_is_virtual());
          s.attr ("put_token", p.put_token());

          ::we::type::property::dump::dump (s, p.properties());

          for (const place_type::token_type& token : p.tokens)
          {
            s.open ("token");
            s.open ("value");
            s.content (token);
            s.close();
            s.close();
          }

          s.close();
        }
      }
    }
  }
}
