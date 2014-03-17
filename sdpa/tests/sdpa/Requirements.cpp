#define BOOST_TEST_MODULE TestRequirements

#include "kvs_setup_fixture.hpp"
#include <utils.hpp>

#include <we/layer.hpp>

#include <boost/test/unit_test.hpp>
#include <sdpa/types.hpp>
#include <fhg/util/random_string.hpp>

BOOST_GLOBAL_FIXTURE (setup_logging)
BOOST_GLOBAL_FIXTURE (KVSSetup)

namespace {
  bool operator==(const we::type::requirement_t& left, const we::type::requirement_t& right)
  {
    return ( left.value() == right.value() ) && ( left.is_mandatory() == right.is_mandatory() );
  }

  const we::type::requirement_t req_A("A", true);
  const we::type::requirement_t req_B("B", true);

  // this workflow has 20 tasks of type A and 10 tasks of type B (see CMakeLists)
  const unsigned int n_total_expectd_activities  = 30;

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
}

class TestAgent : public sdpa::daemon::Agent
{
public:
  TestAgent (const std::string& name)
    : Agent (name, "127.0.0.1", kvs_host(), kvs_port(), sdpa::master_info_list_t(), boost::none)
      , _n_recv_tasks_A(0), _n_recv_tasks_B(0)
  {}

  void submit ( const we::layer::id_type&
              , const we::type::activity_t& activity
              )
  {
    const std::list<we::type::requirement_t> list_req( activity.transition().requirements() );

    BOOST_REQUIRE(list_req.front()==req_A || list_req.front()==req_B);

    boost::unique_lock<boost::mutex> const _ (_mtx_all_submitted);
    if(list_req.front()==req_A) {_n_recv_tasks_A++;}
    if(list_req.front()==req_B) {_n_recv_tasks_B++;}

    if (n_recv_tasks_A() + n_recv_tasks_B() == n_total_expectd_activities )
    {
      _cond_all_submitted.notify_one();
    }
  }

  void wait_all_submitted()
  {
    boost::unique_lock<boost::mutex> const _ (_mtx_all_submitted);
    _cond_all_submitted.wait (_mtx_all_submitted);
  }

  unsigned int n_recv_tasks_A() const { return _n_recv_tasks_A;}
  unsigned int n_recv_tasks_B() const { return _n_recv_tasks_B;}

private:

  boost::mutex _mtx_all_submitted;
  boost::condition_variable_any _cond_all_submitted;
  unsigned int _n_recv_tasks_A;
  unsigned int _n_recv_tasks_B;
};

BOOST_AUTO_TEST_CASE (check_requirements)
{
  TestAgent agent ("agent_0");

  agent.workflowEngine()->submit
    ( fhg::util::random_string()
    , net_with_two_childs_that_require_capabilities
      ( we::type::requirement_t ("A", true), 20
      , we::type::requirement_t ("B", true), 10
      )
    );
  agent.wait_all_submitted();

  BOOST_REQUIRE_EQUAL(agent.n_recv_tasks_A(), 20);
  BOOST_REQUIRE_EQUAL(agent.n_recv_tasks_B(), 10);
}
