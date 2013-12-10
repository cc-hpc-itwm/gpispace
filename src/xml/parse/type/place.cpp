// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/place.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/id/mapper.hpp>
#include <xml/parse/type/net.hpp>

#include <we/type/value/show.hpp>
#include <we/type/signature/is_literal.hpp>
#include <we/type/signature/show.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      place_type::place_type ( ID_CONS_PARAM(place)
                             , PARENT_CONS_PARAM(net)
                             , const util::position_type& pod
                             , const std::string & name
                             , const std::string & type
                             , const boost::optional<bool> is_virtual
                             )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _is_virtual (is_virtual)
        , _name (name)
        , _type (type)
      {
        _id_mapper->put (_id, *this);
      }

      place_type::place_type ( ID_CONS_PARAM(place)
                             , PARENT_CONS_PARAM(net)
                             , const util::position_type& pod
                             , const boost::optional<bool>& is_virtual
                             , const std::string& name
                             , const std::string& type
                             , const std::list<token_type>& tokens
                             , const we::type::property::type& properties
                             )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _is_virtual (is_virtual)
        , _name (name)
        , _type (type)
        , tokens (tokens)
        , _properties (properties)
      {
        _id_mapper->put (_id, *this);
      }

      const std::string& place_type::name() const
      {
        return _name;
      }
      const std::string& place_type::name_impl (const std::string& name)
      {
        return _name = name;
      }
      const std::string& place_type::name (const std::string& name)
      {
        if (has_parent())
        {
          parent()->rename (make_reference_id(), name);
          return _name;
        }

        return name_impl (name);
      }

      const std::string& place_type::type() const
      {
        return _type;
      }
      const std::string& place_type::type (const std::string& type_)
      {
        return _type = type_;
      }

      boost::optional<pnet::type::signature::signature_type>
        place_type::signature() const
      {
        if (pnet::type::signature::is_literal (type()))
        {
          return pnet::type::signature::signature_type (type());
        }

        if (not parent())
        {
          return boost::none;
        }

        return parent()->signature (type());
      }
      pnet::type::signature::signature_type place_type::signature_or_throw() const
      {
        const boost::optional<pnet::type::signature::signature_type> s (signature());

        if (not s)
        {
          throw error::place_type_unknown (make_reference_id());
        }

        return *s;
      }

      void place_type::push_token (const token_type & t)
      {
        tokens.push_back (t);
      }

      void place_type::specialize ( const type::type_map_type & map_in
                                  , const state::type &
                                  )
      {
        const type::type_map_type::const_iterator
          mapped (map_in.find (type()));

        if (mapped != map_in.end())
        {
          type (mapped->second);
        }
      }

      const boost::optional<bool>& place_type::get_is_virtual (void) const
      {
        return _is_virtual;
      }
      bool place_type::is_virtual (void) const
      {
        return _is_virtual.get_value_or (false);
      }
      void place_type::set_virtual (bool is)
      {
        _is_virtual = boost::make_optional (is, true);
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

      id::ref::place place_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return place_type
          ( new_id
          , new_mapper
          , parent
          , _position_of_definition
          , _is_virtual
          , _name
          , _type
          , tokens
          , _properties
          ).make_reference_id();
      }

      namespace dump
      {
        void dump (fhg::util::xml::xmlstream & s, const place_type & p)
        {
          s.open ("place");
          s.attr ("name", p.name());
          s.attr ("type", p.type());
          s.attr ("virtual", p.get_is_virtual());

          ::we::type::property::dump::dump (s, p.properties());

          BOOST_FOREACH (const place_type::token_type& token, p.tokens)
          {
            s.open ("token");
            s.open ("value");
            s.content (pnet::type::value::show (token));
            s.close();
            s.close();
          }

          s.close();
        }
      }
    }
  }
}
