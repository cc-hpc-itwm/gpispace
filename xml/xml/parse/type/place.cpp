// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/place.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/id/mapper.hpp>
#include <xml/parse/type/net.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      place_type::place_type ( ID_CONS_PARAM(place)
                             , PARENT_CONS_PARAM(net)
                             )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
      {
        _id_mapper->put (_id, *this);
      }

      place_type::place_type ( ID_CONS_PARAM(place)
                             , PARENT_CONS_PARAM(net)
                             , const std::string & name
                             , const std::string & _type
                             , const boost::optional<bool> is_virtual
                             )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _is_virtual (is_virtual)
        , _name (name)
        , type (_type)
      {
        _id_mapper->put (_id, *this);
      }

      place_type::place_type ( ID_CONS_PARAM(place)
                             , PARENT_CONS_PARAM(net)
                             , const boost::optional<bool>& is_virtual
                             , const std::string& name
                             , const std::string& type
                             , const std::list<token_type>& tokens
                             , const we::type::property::type& properties
                             )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _is_virtual (is_virtual)
        , _name (name)
        , type (type)
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

      boost::optional<signature::type> place_type::signature() const
      {
        if (literal::valid_name (type_GET()))
        {
          return signature::type (type_GET());
        }

        if (not parent())
        {
          return boost::none;
        }

        return parent()->signature (type_GET());
      }
      signature::type place_type::signature_or_throw() const
      {
        const boost::optional<signature::type> s (signature());

        if (not s)
        {
          throw error::place_type_unknown ( name()
                                          , type_GET()
                                          //! \todo own LOCATION
                                          , parent()->path()
                                          );
        }

        return *s;
      }

      const std::string& place_type::type_GET() const
      {
        return /*! \todo _*/type;
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
          mapped (map_in.find (type));

        if (mapped != map_in.end())
        {
          type = mapped->second;
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
          , _is_virtual
          , _name
          , type
          , tokens
          , _properties
          ).make_reference_id();
      }

      namespace dump
      {
        void dump (xml_util::xmlstream & s, const place_type & p)
        {
          s.open ("place");
          s.attr ("name", p.name());
          s.attr ("type", p.type);
          s.attr ("virtual", p.get_is_virtual());

          ::we::type::property::dump::dump (s, p.properties());

          BOOST_FOREACH (const place_type::token_type& token, p.tokens)
            {
              boost::apply_visitor ( signature::visitor::dump_token ("", s)
                                   , token
                                   );
            }

          s.close();
        }
      }
    }
  }
}
