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
                             , boost::optional<pnet::type::signature::signature_type> signature
                             )
        : with_position_of_definition (pod)
        , _is_virtual (is_virtual)
        , _put_token (put_token)
        , _name (name)
        , _type (type)
        , _signature (std::move (signature))
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
                          , _signature
                          );
      }

      pnet::type::signature::signature_type place_type::signature() const
      {
        //! \note assume post processing pass (resolve_types_recursive)
        return _signature.get();
      }
      void place_type::resolve_types_recursive
        (std::unordered_map<std::string, pnet::type::signature::signature_type> known)
      {
        if (pnet::type::signature::is_literal (_type))
        {
          _signature = pnet::type::signature::signature_type (_type);
        }
        else
        {
          auto it (known.find (_type));
          if (it == known.end())
          {
            throw error::place_type_unknown (*this);
          }
          _signature = it->second;
        }
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
                          , _signature
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
