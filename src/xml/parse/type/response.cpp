#include <xml/parse/type/response.hpp>

#include <xml/parse/type/transition.hpp>
#include <xml/parse/util/position.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      response_type::response_type  ( ID_CONS_PARAM (response)
                                    , PARENT_CONS_PARAM (transition)
                                    , util::position_type const& pod
                                    , std::string const& port
                                    , std::string const& to
                                    , we::type::property::type const& properties
                                    )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _port (port)
        , _to (to)
        , _properties (properties)
      {
        _id_mapper->put (_id, *this);
      }

      id::ref::response response_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return response_type
          ( new_id
          , new_mapper
          , parent
          , _position_of_definition
          , _port
          , _to
          , _properties
          ).make_reference_id();
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, response_type const& r)
        {
          s.open ("connect-response");
          s.attr ("port", r.port());
          s.attr ("to", r.to());

          ::we::type::property::dump::dump (s, r.properties());

          s.close();
        }
      }
    }
  }
}
