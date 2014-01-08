// {rahn,loerwald}@itwm.fraunhofer.de

#ifndef WE_TEST_OPERATOR_EQUAL_HPP
#define WE_TEST_OPERATOR_EQUAL_HPP

#include <we/mgmt/type/activity.hpp>

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
        return lhs.get_map() == rhs.get_map();
      }
    }
  }
}

namespace condition
{
  bool operator== (type const& lhs, type const& rhs)
  {
    return lhs.expression() == rhs.expression();
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

namespace petri_net
{
  namespace
  {
    std::map< petri_net::place_id_type
            , std::list<pnet::type::value::value_type>
            > tokens (net const& n)
    {
      std::map< petri_net::place_id_type
              , std::list<pnet::type::value::value_type>
              > tokens_on_place;

      BOOST_FOREACH ( petri_net::place_id_type const& place_id
                    , n.places() | boost::adaptors::map_keys
                    )
      {
        tokens_on_place.insert
          (std::make_pair (place_id, n.get_token (place_id)));
      }

      return tokens_on_place;
    }
  }

  bool operator== (net const& lhs, net const& rhs)
  {
    return lhs.places() == rhs.places()
      && lhs.transitions() == rhs.transitions()
      && lhs.transition_to_place() == rhs.transition_to_place()
      && lhs.place_to_transition_consume() == rhs.place_to_transition_consume()
      && lhs.place_to_transition_read() == rhs.place_to_transition_read()
      && tokens (lhs) == tokens (rhs);
  }
}

namespace we
{
  namespace type
  {
    bool operator== (port_t const& lhs, port_t const& rhs)
    {
      return lhs.name() == rhs.name()
        && lhs.direction() == rhs.direction()
        && lhs.signature() == rhs.signature()
        && lhs.associated_place() == rhs.associated_place()
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
        && lhs.outer_to_inner() == rhs.outer_to_inner()
        && lhs.inner_to_outer() == rhs.inner_to_outer()
        && lhs.ports() == rhs.ports()
        && lhs.prop() == rhs.prop()
        && lhs.requirements() == rhs.requirements();
    }
  }

  namespace mgmt
  {
    namespace type
    {
      bool operator== (activity_t const& lhs, activity_t const& rhs)
      {
        return lhs.input() == rhs.input()
          && lhs.output() == rhs.output()
          && lhs.transition() == rhs.transition();
      }
    }
  }
}

#endif
