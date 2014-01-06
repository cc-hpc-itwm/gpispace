#define BOOST_TEST_MODULE layer
#include <boost/test/unit_test.hpp>

#include <we/mgmt/layer.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/test/operator_equal.hpp>
#include <we/type/expression.hpp>
#include <we/type/module_call.hpp>
#include <we/type/signature.hpp>
#include <we/type/transition.hpp>
#include <we/type/value/read.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>

#define DECLARE_EXPECT_CLASS(NAME, CTOR_ARGUMENTS, INITIALIZER_LIST, MEMBER_VARIABLES) \
  struct expect_ ## NAME                                                \
  {                                                                     \
    expect_ ## NAME (daemon* d, CTOR_ARGUMENTS)                         \
      : _happened (false)                                               \
      , INITIALIZER_LIST                                                \
    {                                                                   \
      d->_to_ ## NAME.push (this);                                      \
    }                                                                   \
    ~expect_ ## NAME()                                                  \
    {                                                                   \
      boost::mutex::scoped_lock lock (_happened_mutex);                 \
      while (!_happened)                                                \
      {                                                                 \
        _happened_condition.wait (lock);                                \
      }                                                                 \
    }                                                                   \
    void happened()                                                     \
    {                                                                   \
      _happened = true;                                                 \
      _happened_condition.notify_one();                                 \
    }                                                                   \
    bool _happened;                                                     \
    mutable boost::mutex _happened_mutex;                               \
    boost::condition_variable_any _happened_condition;                  \
                                                                        \
    MEMBER_VARIABLES;                                                   \
  };                                                                    \
                                                                        \
  std::stack<expect_ ## NAME*> _to_ ## NAME

struct daemon
{
  daemon()
    : _cnt()
    , layer ( boost::bind (&daemon::submit, this, _1, _2, _3)
            , boost::bind (&daemon::cancel, this, _1)
            , boost::bind (&daemon::finished, this, _1, _2)
            , boost::bind (&daemon::failed, this, _1, _2, _3)
            , boost::bind (&daemon::canceled, this, _1)
            , boost::bind (&daemon::generate_id, this)
            , _random_engine
            )
    , _in_progress()
  {}
  ~daemon()
  {
    boost::mutex::scoped_lock lock (_in_progress_mutex);

    while (_in_progress > 0)
    {
      _in_progress_condition.wait (lock);
    }

    BOOST_REQUIRE (_to_submit.empty());
    BOOST_REQUIRE (_to_cancel.empty());
    BOOST_REQUIRE (_to_finished.empty());
    BOOST_REQUIRE (_to_failed.empty());
    BOOST_REQUIRE (_to_canceled.empty());
  }

  DECLARE_EXPECT_CLASS ( submit
                       , we::mgmt::layer::id_type* id
        BOOST_PP_COMMA() we::mgmt::type::activity_t act
        BOOST_PP_COMMA() we::mgmt::layer::id_type parent
                       , _id (id)
        BOOST_PP_COMMA() _act (act)
        BOOST_PP_COMMA() _parent (parent)
                       , we::mgmt::layer::id_type* _id
                       ; we::mgmt::type::activity_t _act
                       ; we::mgmt::layer::id_type _parent
                       );

  DECLARE_EXPECT_CLASS ( cancel
                       , we::mgmt::layer::id_type id
                       , _id (id)
                       , we::mgmt::layer::id_type _id
                       );

  DECLARE_EXPECT_CLASS ( finished
                       , we::mgmt::layer::id_type id
        BOOST_PP_COMMA() we::mgmt::type::activity_t act
                       , _id (id)
        BOOST_PP_COMMA() _act (act)
                       , we::mgmt::layer::id_type _id
                       ; we::mgmt::type::activity_t _act
                       );

  DECLARE_EXPECT_CLASS ( failed
                       , we::mgmt::layer::id_type id
        BOOST_PP_COMMA() int ec
        BOOST_PP_COMMA() std::string message
                       , _id (id)
        BOOST_PP_COMMA() _ec (ec)
        BOOST_PP_COMMA() _message (message)
                       , we::mgmt::layer::id_type _id
                       ; int _ec
                       ; std::string _message
                       );

  DECLARE_EXPECT_CLASS ( canceled
                       , we::mgmt::layer::id_type id
                       , _id (id)
                       , we::mgmt::layer::id_type _id
                       );

#define INC_IN_PROGRESS                                         \
  {                                                             \
    boost::mutex::scoped_lock const _ (_in_progress_mutex);     \
    ++_in_progress;                                             \
  }

#define DEC_IN_PROGRESS                                         \
  {                                                             \
    boost::mutex::scoped_lock const _ (_in_progress_mutex);     \
    --_in_progress;                                             \
  }                                                             \
  _in_progress_condition.notify_one()

  void submit ( we::mgmt::layer::id_type id
              , we::mgmt::type::activity_t act
              , we::mgmt::layer::id_type parent
              )
  {
    INC_IN_PROGRESS;

    BOOST_REQUIRE (!_to_submit.empty());
    *_to_submit.top()->_id = id;
    BOOST_REQUIRE_EQUAL (_to_submit.top()->_act, act);
    BOOST_REQUIRE_EQUAL (_to_submit.top()->_parent, parent);
    _to_submit.top()->happened();
    _to_submit.pop();
  }

  void cancel (we::mgmt::layer::id_type id)
  {
    INC_IN_PROGRESS;

    BOOST_REQUIRE (!_to_cancel.empty());
    BOOST_REQUIRE_EQUAL (_to_cancel.top()->_id, id);
    _to_cancel.top()->happened();
    _to_cancel.pop();
  }

  void finished ( we::mgmt::layer::id_type id
                , we::mgmt::type::activity_t act
                )
  {
    BOOST_REQUIRE (!_to_finished.empty());
    BOOST_REQUIRE_EQUAL (_to_finished.top()->_id, id);
    BOOST_REQUIRE_EQUAL (_to_finished.top()->_act, act);
    _to_finished.top()->happened();
    _to_finished.pop();

    DEC_IN_PROGRESS;
  }

  void failed ( we::mgmt::layer::id_type id
              , int ec
              , std::string message
              )
  {
    BOOST_REQUIRE (!_to_failed.empty());
    BOOST_REQUIRE_EQUAL (_to_failed.top()->_id, id);
    BOOST_REQUIRE_EQUAL (_to_failed.top()->_ec, ec);
    BOOST_REQUIRE_EQUAL (_to_failed.top()->_message, message);
    _to_failed.top()->happened();
    _to_failed.pop();

    DEC_IN_PROGRESS;
  }

  void canceled (we::mgmt::layer::id_type id)
  {
    BOOST_REQUIRE (!_to_canceled.empty());
    BOOST_REQUIRE_EQUAL (_to_canceled.top()->_id, id);
    _to_canceled.top()->happened();
    _to_canceled.pop();

    DEC_IN_PROGRESS;
  }

  void do_submit ( we::mgmt::layer::id_type id
                 , we::mgmt::type::activity_t act
                 )
  {
    INC_IN_PROGRESS;

    layer.submit (id, act);
  }

  void do_finished ( we::mgmt::layer::id_type id
                   , we::mgmt::type::activity_t act
                   )
  {
    DEC_IN_PROGRESS;

    layer.finished (id, act);
  }

  unsigned long _cnt;
  we::mgmt::layer::id_type generate_id()
  {
    return boost::lexical_cast<we::mgmt::layer::id_type> (++_cnt);
  }

  boost::mt19937 _random_engine;
  we::mgmt::layer layer;
  mutable boost::mutex _in_progress_mutex;
  boost::condition_variable_any _in_progress_condition;
  unsigned long _in_progress;
};

BOOST_FIXTURE_TEST_CASE (expressions_shall_not_be_sumitted_to_rts, daemon)
{
  using pnet::type::signature::signature_type;

  we::type::transition_t transition
    ( "expression"
    , we::type::expression_t ("${out} := ${in} + 1L")
    , condition::type ("true")
    , true
    , we::type::property::type()
    );
  transition.add_port
    (we::type::port_t ( "in"
                      , we::type::PORT_IN
                      , signature_type (std::string ("long"))
                      , we::type::property::type()
                      )
    );
  transition.add_port
    (we::type::port_t ( "out"
                      , we::type::PORT_OUT
                      , signature_type (std::string ("long"))
                      , we::type::property::type()
                      )
    );

  we::mgmt::type::activity_t activity (transition);
  activity.add_input ( transition.input_port_by_name ("in")
                     , pnet::type::value::read ("1L")
                     );

  we::mgmt::layer::id_type const id (generate_id());

  {
    we::mgmt::type::activity_t activity_expected (transition);
    activity_expected.add_output
      ( transition.output_port_by_name ("out")
      , pnet::type::value::read ("2L")
      );

    expect_finished const _ (this, id, activity_expected);

    do_submit (id, activity);
  }
}

namespace
{
  namespace value
  {
    pnet::type::value::value_type const control
      (pnet::type::value::read ("[]"));
  }
  namespace signature
  {
    pnet::type::signature::signature_type control (std::string ("control"));
  }
}

BOOST_FIXTURE_TEST_CASE (module_calls_should_be_submitted_to_rts, daemon)
{
  we::type::transition_t transition
    ( "module call"
    , we::type::module_call_t ("m", "f")
    , condition::type ("true")
    , true
    , we::type::property::type()
    );
  transition.add_port
    (we::type::port_t ( "in"
                      , we::type::PORT_IN
                      , signature::control
                      , we::type::property::type()
                      )
    );
  transition.add_port
    (we::type::port_t ( "out"
                      , we::type::PORT_OUT
                      , signature::control
                      , we::type::property::type()
                      )
    );

  we::mgmt::type::activity_t activity_output (transition);
  activity_output.add_output
    (transition.output_port_by_name ("out"), value::control);

  we::mgmt::type::activity_t activity_input (transition);
  activity_input.add_input
    (transition.input_port_by_name ("in"), value::control);

  //! \todo leaking implementation detail: maybe extract() should
  //! remove those connections to outside that were introduced by
  //! wrap()
  transition.add_connection (1, "in", we::type::property::type());
  transition.add_connection ("out", 0, we::type::property::type());
  we::mgmt::type::activity_t activity_child (transition);
  activity_child.add_input
    (transition.input_port_by_name ("in"), value::control);

  we::mgmt::type::activity_t activity_result (transition);
  activity_result.add_output
    (transition.output_port_by_name ("out"), value::control);

  we::mgmt::layer::id_type const id (generate_id());

  {
    expect_finished const _ (this, id, activity_output);

    we::mgmt::layer::id_type child_id;
    {
      expect_submit const _ (this, &child_id, activity_child, id);

      do_submit (id, activity_input);
    }

    do_finished (child_id, activity_result);
  }
}

namespace
{
  std::pair<we::type::transition_t, we::type::transition_t>
    finished_shall_be_called_after_finished_make_net (bool put_on_input)
  {
    we::type::transition_t transition
      ( "module call"
      , we::type::module_call_t ("m", "f")
      , condition::type ("true")
      , true
      , we::type::property::type()
      );
    transition.add_port ( we::type::port_t ( "in"
                                           , we::type::PORT_IN
                                           , signature::control
                                           , we::type::property::type()
                                           )
                        );
    transition.add_port ( we::type::port_t ( "out"
                                           , we::type::PORT_OUT
                                           , signature::control
                                           , we::type::property::type()
                                           )
                        );

    petri_net::net net;

    petri_net::place_id_type const place_id_in
      (net.add_place (place::type ("in", signature::control)));
    petri_net::place_id_type const place_id_out
      (net.add_place (place::type ("out", signature::control)));

    net.put_value (put_on_input ? place_id_in : place_id_out, value::control);
    net.put_value (put_on_input ? place_id_in : place_id_out, value::control);

    transition.add_connection (place_id_in, "in", we::type::property::type());
    transition.add_connection ("out", place_id_out, we::type::property::type());

    petri_net::transition_id_type const transition_id_A
      (net.add_transition (transition));
    petri_net::transition_id_type const transition_id_B
      (net.add_transition (transition));

    net.add_connection (petri_net::edge::TP, transition_id_A, place_id_out);
    net.add_connection (petri_net::edge::TP, transition_id_B, place_id_out);
    net.add_connection (petri_net::edge::PT, transition_id_A, place_id_in);
    net.add_connection (petri_net::edge::PT, transition_id_B, place_id_in);

    return std::make_pair ( we::type::transition_t ( "net"
                                                   , net
                                                   , condition::type ("true")
                                                   , true
                                                   , we::type::property::type()
                                                   )
                          , transition
                          );
  }
}

BOOST_FIXTURE_TEST_CASE (finished_shall_be_called_after_finished, daemon)
{
  we::type::transition_t transition_in;
  we::type::transition_t transition_out;
  we::type::transition_t transition_child;
  boost::tie (transition_in, transition_child) =
    finished_shall_be_called_after_finished_make_net (true);
  boost::tie (transition_out, boost::tuples::ignore) =
    finished_shall_be_called_after_finished_make_net (false);

  we::mgmt::type::activity_t activity_input (transition_in);
  we::mgmt::type::activity_t activity_output (transition_out);

  we::mgmt::type::activity_t activity_child (transition_child);
  activity_child.add_input
    (transition_child.input_port_by_name ("in"), value::control);

  we::mgmt::type::activity_t activity_result (transition_child);
  activity_result.add_output
    (transition_child.output_port_by_name ("out"), value::control);

  we::mgmt::layer::id_type const id (generate_id());

  we::mgmt::layer::id_type child_id_A;
  we::mgmt::layer::id_type child_id_B;

  {
    expect_submit const _A (this, &child_id_A, activity_child, id);
    expect_submit const _B (this, &child_id_B, activity_child, id);

    do_submit (id, activity_input);
  }

  do_finished (child_id_A, activity_result);

  {
    expect_finished const _ (this, id, activity_output);

    do_finished (child_id_B, activity_result);
  }
}

// BOOST_AUTO_TEST_CASE (canceled_shall_be_called_after_cancel)
// BOOST_AUTO_TEST_CASE (child_jobs_shall_be_canceled_after_cancel)
