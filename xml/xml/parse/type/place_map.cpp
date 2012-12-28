// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/place_map.hpp>

#include <xml/parse/id/mapper.hpp>
#include <xml/parse/type/transition.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      place_map_type::place_map_type ( ID_CONS_PARAM(place_map)
                                     , PARENT_CONS_PARAM(transition)
                                     , const std::string & place_virtual
                                     , const std::string & place_real
                                     , const we::type::property::type& prop
                                     )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _place_virtual (place_virtual)
        , _place_real (place_real)
        , _properties (prop)
      {
        _id_mapper->put (_id, *this);
      }

      const std::string& place_map_type::place_virtual() const
      {
        return _place_virtual;
      }
      const std::string& place_map_type::place_real() const
      {
        return _place_real;
      }
      const std::string& place_map_type::place_real_impl (const std::string& v)
      {
        return _place_real = v;
      }
      const std::string& place_map_type::place_real (const std::string& v)
      {
        if (has_parent())
        {
          parent()->place_map_real (make_reference_id(), v);
          return _place_real;
        }
        return place_real_impl (v);
      }

      const we::type::property::type& place_map_type::properties() const
      {
        return _properties;
      }

      place_map_type::unique_key_type place_map_type::unique_key() const
      {
        return std::make_pair (place_virtual(), place_real());
      }

      id::ref::place_map place_map_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return place_map_type
          ( new_id
          , new_mapper
          , parent
          , _place_virtual
          , _place_real
          , _properties
          ).make_reference_id();
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const place_map_type & p
                  )
        {
          s.open ("place-map");
          s.attr ("virtual", p.place_virtual());
          s.attr ("real", p.place_real());

          ::we::type::property::dump::dump (s, p.properties());

          s.close ();
        }
      }
    }
  }
}
