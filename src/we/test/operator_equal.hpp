// {rahn,loerwald}@itwm.fraunhofer.de

#ifndef WE_TEST_OPERATOR_EQUAL_HPP
#define WE_TEST_OPERATOR_EQUAL_HPP

#include <we/type/activity.hpp>

#include <we/type/place.hpp>
#include <we/type/net.hpp>

#include <boost/foreach.hpp>

namespace we
{
  namespace type
  {
    namespace property
    {
      bool operator== (type const& lhs, type const& rhs)
      {
        return lhs.list() == rhs.list();
      }
    }
  }
}

namespace place
{
  bool operator== (type const& lhs, type const& rhs)
  {
    return lhs.name() == rhs.name()
      && lhs.signature() == rhs.signature()
      && lhs.property() == rhs.property();
  }
}

namespace we
{
  namespace type
  {
    namespace
    {
      std::map< we::place_id_type
              , std::list<pnet::type::value::value_type>
              > tokens (net_type const& n)
      {
        std::map< we::place_id_type
                , std::list<pnet::type::value::value_type>
                > tokens_on_place;

        BOOST_FOREACH ( we::place_id_type const& place_id
                      , n.places() | boost::adaptors::map_keys
                      )
        {
          tokens_on_place.insert
            (std::make_pair (place_id, n.get_token (place_id)));
        }

        return tokens_on_place;
      }
    }

    bool operator== (net_type const& lhs, net_type const& rhs)
    {
      return lhs.places() == rhs.places()
        && lhs.transitions() == rhs.transitions()
        && lhs.transition_to_place() == rhs.transition_to_place()
        && lhs.place_to_transition_consume() == rhs.place_to_transition_consume()
        && lhs.place_to_transition_read() == rhs.place_to_transition_read()
        && lhs.port_to_place() == rhs.port_to_place()
        && lhs.place_to_port() == rhs.place_to_port()
        && tokens (lhs) == tokens (rhs);
    }

    bool operator== (port_t const& lhs, port_t const& rhs)
    {
      return lhs.name() == rhs.name()
        && lhs.direction() == rhs.direction()
        && lhs.signature() == rhs.signature()
        && lhs.property() == rhs.property();
    }

    bool operator== (requirement_t const& lhs, requirement_t const& rhs)
    {
      return lhs.value() == rhs.value()
        && lhs.is_mandatory() == rhs.is_mandatory();
    }

    bool operator== (module_call_t const& lhs, module_call_t const& rhs)
    {
      return lhs.module() == rhs.module()
        && lhs.function() == rhs.function();
    }

    bool operator== (expression_t const& lhs, expression_t const& rhs)
    {
      return lhs.expression() == rhs.expression();
    }

    bool operator== (transition_t const& lhs, transition_t const& rhs)
    {
      return lhs.name() == rhs.name()
        && lhs.data() == rhs.data()
        && lhs.is_internal() == rhs.is_internal()
        && lhs.condition() == rhs.condition()
        && lhs.ports_input() == rhs.ports_input()
        && lhs.ports_output() == rhs.ports_output()
        && lhs.ports_tunnel() == rhs.ports_tunnel()
        && lhs.prop() == rhs.prop()
        && lhs.requirements() == rhs.requirements();
    }
  }

    namespace type
    {
      bool operator== (activity_t const& lhs, activity_t const& rhs)
      {
        return lhs.input() == rhs.input()
          && lhs.output() == rhs.output()
          && lhs.transition() == rhs.transition()
          && lhs.transition_id() == rhs.transition_id();
      }
    }
}

#endif
