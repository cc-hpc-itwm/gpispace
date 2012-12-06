// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/net.hpp>

#include <xml/parse/id/mapper.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      connect_type::connect_type ( ID_CONS_PARAM(connect)
                                 , PARENT_CONS_PARAM(transition)
                                 , const std::string& place
                                 , const std::string& port
                                 , const ::petri_net::edge::type& direction
                                 , const we::type::property::type& properties
                                 )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _place (place)
        , _port (port)
        , _direction (direction)
        , _properties (properties)
      {
        _id_mapper->put (_id, *this);
      }

      const std::string& connect_type::place() const
      {
        return _place;
      }
      const std::string& connect_type::port() const
      {
        return _port;
      }

      boost::optional<const id::ref::place&> connect_type::resolved_place() const
      {
        return parent()->parent()->places().get (place());
      }
      boost::optional<const id::ref::port&> connect_type::resolved_port() const
      {
        const id::ref::function fun (parent()->resolved_function());

        //! \note We need to take the correct port, depending on our
        //! direction. Our direction is stored in the parent though. Yay!

        const id::ref::connect this_id (make_reference_id());
        if (parent()->has_in (this_id) || parent()->has_read (this_id))
        {
          return fun.get().in().get (port());
        }
        else if (parent()->has_out (this_id))
        {
          return fun.get().out().get (port());
        }

        throw std::runtime_error
          ("connection that is not in any of the parent's lists");
      }

      const ::petri_net::edge::type& connect_type::direction() const
      {
        return _direction;
      }
      const ::petri_net::edge::type& connect_type::direction
        (const ::petri_net::edge::type& direction_)
      {
        return _direction = direction_;
      }

      const std::string& connect_type::place (const std::string& place)
      {
        return _place = place;
      }

      const we::type::property::type& connect_type::properties() const
      {
        return _properties;
      }
      we::type::property::type& connect_type::properties()
      {
        return _properties;
      }

      connect_type::unique_key_type connect_type::unique_key() const
      {
        return std::make_pair (_place, _port);
      }


      id::ref::connect connect_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return connect_type
          ( new_id
          , new_mapper
          , parent
          , _place
          , _port
          , _direction
          , _properties
          ).make_reference_id();
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const connect_type & c
                  , const std::string & type
                  )
        {
          s.open ("connect-" + type);
          s.attr ("port", c.port());
          s.attr ("place", c.place());

          ::we::type::property::dump::dump (s, c.properties());

          s.close ();
        }
      }
    }
  }
}
