#define BOOST_TEST_MODULE TestRequirements

#include "kvs_setup_fixture.hpp"
#include <utils.hpp>

#include <we/layer.hpp>

#include <boost/test/unit_test.hpp>
#include <sdpa/id_generator.hpp>
#include <sdpa/types.hpp>
#include <fhg/util/random_string.hpp>

namespace we
{
  namespace type
  {
    bool operator== (const requirement_t& left, const requirement_t& right)
    {
      return (left.value() == right.value())
        && (left.is_mandatory() == right.is_mandatory());
    }

    std::size_t hash_value (const requirement_t& requirement)
    {
      std::size_t seed (0);
      boost::hash_combine (seed, requirement.value());
      boost::hash_combine (seed, requirement.is_mandatory());
      return seed;
    }
  }
}
namespace
{
  const we::type::requirement_t req_A ("A", true);
  const we::type::requirement_t req_B ("B", true);

  we::place_id_type add_transition_with_requirement_and_input_place
    (we::type::net_type& net, we::type::requirement_t const& requirement)
  {
    we::type::transition_t transition
      ( fhg::util::random_string()
      , we::type::module_call_t
        (fhg::util::random_string(), fhg::util::random_string())
      , boost::none
      , true
      , we::type::property::type()
      , we::priority_type()
      );
    transition.add_requirement (requirement);

    const std::string port_name (fhg::util::random_string());
    we::port_id_type const port_id
      ( transition.add_port ( we::type::port_t ( port_name
                                               , we::type::PORT_IN
                                               , std::string ("control")
                                               , we::type::property::type()
                                               )
                            )
      );

    we::transition_id_type const transition_id
      (net.add_transition (transition));

    we::place_id_type const place_id
      (net.add_place (place::type (port_name, std::string ("control"))));

    net.add_connection ( we::edge::PT
                       , transition_id
                       , place_id
                       , port_id
                       , we::type::property::type()
                       );

    return place_id;
  }

  we::type::activity_t net_with_two_childs_that_require_capabilities
    ( we::type::requirement_t const& capability_A
    , std::size_t num_worker_with_capability_A
    , we::type::requirement_t const& capability_B
    , std::size_t num_worker_with_capability_B
    )
  {
    we::type::net_type net;

    {
      we::place_id_type const place_id
        (add_transition_with_requirement_and_input_place (net, capability_A));

      while (num_worker_with_capability_A --> 0)
      {
        net.put_value (place_id, we::type::literal::control());
      }
    }

    {
      we::place_id_type const place_id
        (add_transition_with_requirement_and_input_place (net, capability_B));

      while (num_worker_with_capability_B --> 0)
      {
        net.put_value (place_id, we::type::literal::control());
      }
    }

    return we::type::activity_t
      ( we::type::transition_t ( fhg::util::random_string()
                               , net
                               , boost::none
                               , true
                               , we::type::property::type()
                               , we::priority_type()
                               )
      , boost::none
      );
  }

  void disallow (std::string what)
  {
    throw std::runtime_error ("disallowed function called: " + what);
  }
}

class wfe_and_counter_of_submitted_requirements
{
public:
  wfe_and_counter_of_submitted_requirements (unsigned int expected_activities)
    : _expected_activities (expected_activities)
    , _received_requirements()
    , _random_extraction_engine()
    , _id_gen ("job")
    , _layer ( boost::bind
               (&wfe_and_counter_of_submitted_requirements::submit, this, _2)
             , boost::bind (&disallow, "cancel")
             , boost::bind (&disallow, "finished")
             , boost::bind (&disallow, "failed")
             , boost::bind (&disallow, "canceled")
             , boost::bind (&disallow, "discover")
             , boost::bind (&disallow, "discovered")
             , boost::bind (&sdpa::id_generator::next, &_id_gen)
             , _random_extraction_engine
             )
  {}

  void submit (const we::type::activity_t& activity)
  {
    const std::list<we::type::requirement_t> list_req
      (activity.transition().requirements());

    BOOST_REQUIRE_EQUAL (list_req.size(), 1);

    boost::unique_lock<boost::mutex> const _ (_mtx_all_submitted);
    ++_received_requirements[list_req.front()];

    if (--_expected_activities == 0)
    {
      _cond_all_submitted.notify_one();
    }
  }

  void wait_all_submitted()
  {
    boost::unique_lock<boost::mutex> const _ (_mtx_all_submitted);
    _cond_all_submitted.wait (_mtx_all_submitted);
  }

private:
  boost::mutex _mtx_all_submitted;
  boost::condition_variable_any _cond_all_submitted;
  unsigned int _expected_activities;

public:
  boost::unordered_map<we::type::requirement_t, unsigned int>
    _received_requirements;

private:
  boost::mt19937 _random_extraction_engine;
  sdpa::id_generator _id_gen;

public:
  we::layer _layer;
};

BOOST_AUTO_TEST_CASE (check_requirements)
{
  wfe_and_counter_of_submitted_requirements agent (30);

  agent._layer.submit
    ( fhg::util::random_string()
    , net_with_two_childs_that_require_capabilities (req_A, 20, req_B, 10)
    );
  agent.wait_all_submitted();

  BOOST_REQUIRE_EQUAL (agent._received_requirements.size(), 2);
  BOOST_REQUIRE_EQUAL (agent._received_requirements.at (req_A), 20);
  BOOST_REQUIRE_EQUAL (agent._received_requirements.at (req_B), 10);
}
