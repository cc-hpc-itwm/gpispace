#define BOOST_TEST_MODULE layer
#include <boost/test/unit_test.hpp>

#include <we/layer.hpp>
#include <we/type/activity.hpp>
#include <we/test/operator_equal.hpp>
#include <we/type/expression.hpp>
#include <we/type/module_call.hpp>
#include <we/type/signature.hpp>
#include <we/type/transition.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/poke.hpp>

#include <fhg/util/now.hpp>
#include <fhg/util/random_string.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <functional>
#include <list>
#include <tuple>

#define DECLARE_EXPECT_CLASS(NAME, CTOR_ARGUMENTS, INITIALIZER_LIST, MEMBER_VARIABLES, EQ_IMPL) \
  struct expect_ ## NAME                                                \
  {                                                                     \
    expect_ ## NAME (daemon* d, CTOR_ARGUMENTS)                         \
      : _happened (false)                                               \
      , INITIALIZER_LIST                                                \
    {                                                                   \
      d->_to_ ## NAME.push_back (this);                                 \
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
    bool eq (CTOR_ARGUMENTS) const                                      \
    {                                                                   \
      return EQ_IMPL;                                                   \
    }                                                                   \
    bool _happened;                                                     \
    mutable boost::mutex _happened_mutex;                               \
    boost::condition_variable_any _happened_condition;                  \
                                                                        \
    MEMBER_VARIABLES;                                                   \
  };                                                                    \
                                                                        \
  std::list<expect_ ## NAME*> _to_ ## NAME

struct daemon
{
  daemon()
    : _cnt()
    , layer ( std::bind (&daemon::submit, this, std::placeholders::_1, std::placeholders::_2)
            , std::bind (&daemon::cancel, this, std::placeholders::_1)
            , std::bind (&daemon::finished, this, std::placeholders::_1, std::placeholders::_2)
            , std::bind (&daemon::failed, this, std::placeholders::_1, std::placeholders::_2)
            , std::bind (&daemon::canceled, this, std::placeholders::_1)
            , std::bind (&daemon::discover, this, std::placeholders::_1, std::placeholders::_2)
            , std::bind (&daemon::discovered, this, std::placeholders::_1, std::placeholders::_2)
            , std::bind (&daemon::generate_id, this)
            , _random_engine
            )
    , _in_progress_jobs_rts()
    , _in_progress_jobs_layer()
    , _in_progress_replies()
  {}
  ~daemon()
  {
    boost::mutex::scoped_lock lock (_in_progress_mutex);

    while ( _in_progress_jobs_rts
          + _in_progress_jobs_layer
          + _in_progress_replies
          > 0
          )
    {
      _in_progress_condition.wait (lock);
    }

    BOOST_REQUIRE (_to_submit.empty());
    BOOST_REQUIRE (_to_cancel.empty());
    BOOST_REQUIRE (_to_finished.empty());
    BOOST_REQUIRE (_to_failed.empty());
    BOOST_REQUIRE (_to_canceled.empty());
  }

#define INC_IN_PROGRESS(COUNTER)                                \
  {                                                             \
    boost::mutex::scoped_lock const _ (_in_progress_mutex);     \
    ++_in_progress_ ## COUNTER;                                 \
  }

#define DEC_IN_PROGRESS(COUNTER)                                \
  {                                                             \
    boost::mutex::scoped_lock const _ (_in_progress_mutex);     \
    --_in_progress_ ## COUNTER;                                 \
  }                                                             \
  _in_progress_condition.notify_one()

  DECLARE_EXPECT_CLASS ( submit
                       , we::layer::id_type* id
        BOOST_PP_COMMA() we::type::activity_t act
                       , _id (id)
        BOOST_PP_COMMA() _act (act)
                       , we::layer::id_type* _id
                       ; we::type::activity_t _act
                       , id != nullptr && _act == act
                       );

  void submit
    (we::layer::id_type id, we::type::activity_t act)
  {
    INC_IN_PROGRESS (jobs_rts);

    std::list<expect_submit*>::iterator const e
      ( boost::find_if ( _to_submit
                       , std::bind (&expect_submit::eq, std::placeholders::_1, &id, act)
                       )
        );

    BOOST_REQUIRE (e != _to_submit.end());

    *((*e)->_id) = id;

    (*e)->happened();
    _to_submit.erase (e);
  }

  DECLARE_EXPECT_CLASS ( cancel
                       , we::layer::id_type id
                       , _id (id)
                       , we::layer::id_type _id
                       , _id == id
                       );

  void cancel (we::layer::id_type id)
  {
    INC_IN_PROGRESS (replies);

    std::list<expect_cancel*>::iterator const e
      ( boost::find_if ( _to_cancel
                       , std::bind (&expect_cancel::eq, std::placeholders::_1, id)
                       )
        );

    BOOST_REQUIRE (e != _to_cancel.end());

    (*e)->happened();
    _to_cancel.erase (e);
  }


  DECLARE_EXPECT_CLASS ( finished
                       , we::layer::id_type id
        BOOST_PP_COMMA() we::type::activity_t act
                       , _id (id)
        BOOST_PP_COMMA() _act (act)
                       , we::layer::id_type _id
                       ; we::type::activity_t _act
                       , _id == id && _act == act
                       );

  void finished ( we::layer::id_type id
                , we::type::activity_t act
                )
  {
    std::list<expect_finished*>::iterator const e
      ( boost::find_if ( _to_finished
                       , std::bind (&expect_finished::eq, std::placeholders::_1, id, act)
                       )
      );

    BOOST_REQUIRE (e != _to_finished.end());

    (*e)->happened();
    _to_finished.erase (e);

    DEC_IN_PROGRESS (jobs_layer);
  }

  DECLARE_EXPECT_CLASS ( failed
                       , we::layer::id_type id
        BOOST_PP_COMMA() std::string message
                       , _id (id)
        BOOST_PP_COMMA() _message (message)
                       , we::layer::id_type _id
                       ; std::string _message
                       , _id == id && _message == message
                       );

  void failed (we::layer::id_type id, std::string message)
  {
    std::list<expect_failed*>::iterator const e
      ( boost::find_if ( _to_failed
                       , std::bind (&expect_failed::eq, std::placeholders::_1, id, message)
                       )
      );

    BOOST_REQUIRE (e != _to_failed.end());

    (*e)->happened();
    _to_failed.erase (e);

    DEC_IN_PROGRESS (jobs_layer);
  }

  DECLARE_EXPECT_CLASS ( canceled
                       , we::layer::id_type id
                       , _id (id)
                       , we::layer::id_type _id
                       , _id == id
                       );

  void canceled (we::layer::id_type id)
  {
    std::list<expect_canceled*>::iterator const e
      ( boost::find_if ( _to_canceled
                       , std::bind (&expect_canceled::eq, std::placeholders::_1, id)
                       )
      );

    BOOST_REQUIRE (e != _to_canceled.end());

    (*e)->happened();
    _to_canceled.erase (e);

    DEC_IN_PROGRESS (jobs_layer);
    DEC_IN_PROGRESS (replies);
  }

  DECLARE_EXPECT_CLASS ( discover
                       , we::layer::id_type discover_id
        BOOST_PP_COMMA() we::layer::id_type id
                       , _discover_id (discover_id)
        BOOST_PP_COMMA() _id (id)
                       , we::layer::id_type _discover_id
                       ; we::layer::id_type _id
                       , _discover_id == discover_id && _id == id
                       );

  void discover
    (we::layer::id_type discover_id, we::layer::id_type id)
  {
    std::list<expect_discover*>::iterator const e
      ( boost::find_if ( _to_discover
                       , std::bind (&expect_discover::eq, std::placeholders::_1, discover_id, id)
                       )
      );

    BOOST_REQUIRE (e != _to_discover.end());

    (*e)->happened();
    _to_discover.erase (e);

    INC_IN_PROGRESS (replies);
  }

  DECLARE_EXPECT_CLASS ( discovered
                       , we::layer::id_type discover_id
        BOOST_PP_COMMA() sdpa::discovery_info_t result
                       , _discover_id (discover_id)
        BOOST_PP_COMMA() _result (result)
                       , we::layer::id_type _discover_id
                       ; sdpa::discovery_info_t _result
                       , _discover_id == discover_id && _result == result
                       );

  void discovered ( we::layer::id_type discover_id
                  , sdpa::discovery_info_t result
                  )
  {
    std::list<expect_discovered*>::iterator const e
      ( boost::find_if ( _to_discovered
                       , std::bind (&expect_discovered::eq, std::placeholders::_1, discover_id, result)
                       )
      );

    BOOST_REQUIRE (e != _to_discovered.end());

    (*e)->happened();
    _to_discovered.erase (e);

    DEC_IN_PROGRESS (replies);
  }

  void do_submit ( we::layer::id_type id
                 , we::type::activity_t act
                 )
  {
    INC_IN_PROGRESS (jobs_layer);

    layer.submit (id, act);
  }

  void do_finished ( we::layer::id_type id
                   , we::type::activity_t act
                   )
  {
    DEC_IN_PROGRESS (jobs_rts);

    layer.finished (id, act);
  }

  void do_cancel (we::layer::id_type id)
  {
    INC_IN_PROGRESS (replies);

    layer.cancel (id);
  }

  void do_failed (we::layer::id_type id, std::string message)
  {
    DEC_IN_PROGRESS (jobs_rts);

    layer.failed (id, message);
  }

  void do_canceled (we::layer::id_type id)
  {
    DEC_IN_PROGRESS (replies);
    DEC_IN_PROGRESS (jobs_rts);

    layer.canceled (id);
  }

  void do_discover
    (we::layer::id_type discover_id, we::layer::id_type id)
  {
    INC_IN_PROGRESS (replies);

    layer.discover (discover_id, id);
  }
  void do_discovered
    (we::layer::id_type discover_id, sdpa::discovery_info_t result)
  {
    DEC_IN_PROGRESS (replies);

    layer.discovered (discover_id, result);
  }

  boost::mutex _generate_id_mutex;
  unsigned long _cnt;
  we::layer::id_type generate_id()
  {
    boost::mutex::scoped_lock const _ (_generate_id_mutex);
    return boost::lexical_cast<we::layer::id_type> (++_cnt);
  }

  std::mt19937 _random_engine;
  we::layer layer;
  mutable boost::mutex _in_progress_mutex;
  boost::condition_variable_any _in_progress_condition;
  unsigned long _in_progress_jobs_rts;
  unsigned long _in_progress_jobs_layer;
  unsigned long _in_progress_replies;
};

namespace
{
  namespace value
  {
    pnet::type::value::value_type const CONTROL
      (pnet::type::value::read ("[]"));
  }
  namespace signature
  {
    pnet::type::signature::signature_type CONTROL (std::string ("control"));
    pnet::type::signature::signature_type LONG (std::string ("long"));
  }
}

BOOST_FIXTURE_TEST_CASE (expressions_shall_not_be_sumitted_to_rts, daemon)
{
  we::type::transition_t transition
    ( "expression"
    , we::type::expression_t ("${out} := ${in} + 1L")
    , boost::none
    , true
    , we::type::property::type()
    , we::priority_type()
    );
  transition.add_port
    (we::type::port_t ( "in"
                      , we::type::PORT_IN
                      , signature::LONG
                      , we::type::property::type()
                      )
    );
  transition.add_port
    (we::type::port_t ( "out"
                      , we::type::PORT_OUT
                      , signature::LONG
                      , we::type::property::type()
                      )
    );

  we::type::activity_t activity (transition, boost::none);
  activity.add_input ( transition.input_port_by_name ("in")
                     , pnet::type::value::read ("1L")
                     );

  we::layer::id_type const id (generate_id());

  {
    we::type::activity_t activity_expected (transition, boost::none);
    activity_expected.add_output
      ( transition.output_port_by_name ("out")
      , pnet::type::value::read ("2L")
      );

    expect_finished const _ (this, id, activity_expected);

    do_submit (id, activity);
  }
}

BOOST_FIXTURE_TEST_CASE (module_calls_should_be_submitted_to_rts, daemon)
{
  we::type::transition_t transition
    ( "module call"
    , we::type::module_call_t ("m", "f")
    , boost::none
    , true
    , we::type::property::type()
    , we::priority_type()
    );
  transition.add_port ( we::type::port_t ( "in"
                                         , we::type::PORT_IN
                                         , signature::CONTROL
                                         , we::type::property::type()
                                         )
                      );
  transition.add_port ( we::type::port_t ( "out"
                                         , we::type::PORT_OUT
                                         , signature::CONTROL
                                         , we::type::property::type()
                                         )
                      );

  we::type::activity_t activity_output (transition, boost::none);
  activity_output.add_output
    (transition.output_port_by_name ("out"), value::CONTROL);

  we::type::activity_t activity_input (transition, boost::none);
  activity_input.add_input
    (transition.input_port_by_name ("in"), value::CONTROL);

  we::type::activity_t activity_child (transition, we::transition_id_type (0));
  activity_child.add_input
    (transition.input_port_by_name ("in"), value::CONTROL);

  we::type::activity_t activity_result (transition, we::transition_id_type (0));
  activity_result.add_output
    (transition.output_port_by_name ("out"), value::CONTROL);

  we::layer::id_type const id (generate_id());

  {
    expect_finished const _ (this, id, activity_output);

    we::layer::id_type child_id;
    {
      expect_submit const _ (this, &child_id, activity_child);

      do_submit (id, activity_input);
    }

    do_finished (child_id, activity_result);
  }
}

namespace
{
  std::tuple< we::type::transition_t
            , we::type::transition_t
            , we::transition_id_type
            >
    net_with_childs (bool put_on_input, std::size_t token_count)
  {
    we::type::transition_t transition
      ( "module call"
      , we::type::module_call_t ("m", "f")
      , boost::none
      , true
      , we::type::property::type()
      , we::priority_type()
      );
    we::port_id_type const port_id_in
      ( transition.add_port ( we::type::port_t ( "in"
                                               , we::type::PORT_IN
                                               , signature::CONTROL
                                               , we::type::property::type()
                                               )
                            )
      );
    we::port_id_type const port_id_out
      ( transition.add_port ( we::type::port_t ( "out"
                                               , we::type::PORT_OUT
                                               , signature::CONTROL
                                               , we::type::property::type()
                                               )
                            )
      );

    we::type::net_type net;

    we::place_id_type const place_id_in
      (net.add_place (place::type ("in", signature::CONTROL)));
    we::place_id_type const place_id_out
      (net.add_place (place::type ("out", signature::CONTROL)));

    for (std::size_t i (0); i < token_count; ++i)
    {
      net.put_value (put_on_input ? place_id_in : place_id_out, value::CONTROL);
    }

    we::transition_id_type const transition_id
      (net.add_transition (transition));

    {
      using we::edge::TP;
      using we::edge::PT;
      we::type::property::type empty;

      net.add_connection (TP, transition_id, place_id_out, port_id_out, empty);
      net.add_connection (PT, transition_id, place_id_in, port_id_in, empty);
    }

    return std::make_tuple
      ( we::type::transition_t ( "net"
                               , net
                               , boost::none
                               , true
                               , we::type::property::type()
                               , we::priority_type()
                               )
      , transition
      , transition_id
      );
  }

  std::tuple< we::type::activity_t
            , we::type::activity_t
            , we::type::activity_t
            , we::type::activity_t
            >
    activity_with_child (std::size_t token_count)
  {
    we::transition_id_type transition_id_child;
    we::type::transition_t transition_in;
    we::type::transition_t transition_out;
    we::type::transition_t transition_child;
    std::tie (transition_in, transition_child, transition_id_child) =
      net_with_childs (true, token_count);
    std::tie (transition_out, std::ignore, std::ignore) =
      net_with_childs (false, token_count);

    we::type::activity_t activity_input (transition_in, boost::none);
    we::type::activity_t activity_output (transition_out, boost::none);

    we::type::activity_t activity_child
      (transition_child, transition_id_child);
    activity_child.add_input
      (transition_child.input_port_by_name ("in"), value::CONTROL);

    we::type::activity_t activity_result
      (transition_child, transition_id_child);
    activity_result.add_output
      (transition_child.output_port_by_name ("out"), value::CONTROL);

    return std::make_tuple
      (activity_input, activity_output, activity_child, activity_result);
  }
}

BOOST_FIXTURE_TEST_CASE
  (finished_shall_be_called_after_finished_one_child, daemon)
{
  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (1);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id;

  {
    expect_submit const _ (this, &child_id, activity_child);

    do_submit (id, activity_input);
  }

  {
    expect_finished const _ (this, id, activity_output);

    do_finished (child_id, activity_result);
  }
}

BOOST_FIXTURE_TEST_CASE
  (finished_shall_be_called_after_finished_two_childs, daemon)
{
  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (2);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id_A;
  we::layer::id_type child_id_B;

  {
    expect_submit const _A (this, &child_id_A, activity_child);
    expect_submit const _B (this, &child_id_B, activity_child);

    do_submit (id, activity_input);
  }

  do_finished (child_id_A, activity_result);

  {
    expect_finished const _ (this, id, activity_output);

    //! \note There is a race here where layer may call rts_finished
    //! before do_finished, but this is checked by comparing the
    //! output activity: if it would finish before the second child
    //! finishing, there would be a token missing.

    do_finished (child_id_B, activity_result);
  }
}

BOOST_FIXTURE_TEST_CASE
  (two_sequential_jobs_shall_be_properly_handled, daemon)
{
  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (1);

  {
    we::layer::id_type const id (generate_id());

    we::layer::id_type child_id;

    {
      expect_submit const _ (this, &child_id, activity_child);

      do_submit (id, activity_input);
    }

    {
      expect_finished const _ (this, id, activity_output);

      do_finished (child_id, activity_result);
    }
  }
  {
    we::layer::id_type const id (generate_id());

    we::layer::id_type child_id;

    {
      expect_submit const _ (this, &child_id, activity_child);

      do_submit (id, activity_input);
    }

    {
      expect_finished const _ (this, id, activity_output);

      do_finished (child_id, activity_result);
    }
  }
}

BOOST_FIXTURE_TEST_CASE
  (two_interleaving_jobs_shall_be_properly_handled, daemon)
{
  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (2);

  we::layer::id_type const id_0 (generate_id());
  we::layer::id_type const id_1 (generate_id());

  we::layer::id_type child_id_0_0;
  we::layer::id_type child_id_0_1;
  we::layer::id_type child_id_1_0;
  we::layer::id_type child_id_1_1;

  {
    expect_submit const _0 (this, &child_id_0_0, activity_child);
    expect_submit const _1 (this, &child_id_0_1, activity_child);

    do_submit (id_0, activity_input);
  }

  {
    expect_submit const _0 (this, &child_id_1_0, activity_child);
    expect_submit const _1 (this, &child_id_1_1, activity_child);

    do_submit (id_1, activity_input);
  }

  do_finished (child_id_1_1, activity_result);
  do_finished (child_id_0_0, activity_result);

  {
    expect_finished const _ (this, id_1, activity_output);

    do_finished (child_id_1_0, activity_result);
  }

  {
    expect_finished const _ (this, id_0, activity_output);

    do_finished (child_id_0_1, activity_result);
  }
}

BOOST_FIXTURE_TEST_CASE
  (canceled_shall_be_called_after_cancel_one_child, daemon)
{
  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (1);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id;

  {
    expect_submit const _ (this, &child_id, activity_child);

    do_submit (id, activity_input);
  }

  {
    expect_cancel const _ (this, child_id);

    do_cancel (id);
  }

  {
    expect_canceled const _ (this, id);

    do_canceled (child_id);
  }
}

BOOST_FIXTURE_TEST_CASE
  (canceled_shall_be_called_after_cancel_two_childs, daemon)
{
  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (2);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id_A;
  we::layer::id_type child_id_B;

  {
    expect_submit const _A (this, &child_id_A, activity_child);
    expect_submit const _B (this, &child_id_B, activity_child);

    do_submit (id, activity_input);
  }

  {
    expect_cancel const _A (this, child_id_A);
    expect_cancel const _B (this, child_id_B);

    do_cancel (id);
  }

  do_canceled (child_id_A);

  {
    expect_canceled const _ (this, id);

    //! \todo There is an uncheckable(?) race here: rts_canceled may
    //! be called before do_canceled (second child)!

    do_canceled (child_id_B);
  }
}

BOOST_FIXTURE_TEST_CASE
  ( canceled_shall_be_called_after_cancel_two_childs_with_one_child_finished
  , daemon
  )
{
  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (2);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id_A;
  we::layer::id_type child_id_B;

  {
    expect_submit const _A (this, &child_id_A, activity_child);
    expect_submit const _B (this, &child_id_B, activity_child);

    do_submit (id, activity_input);
  }

  do_finished (child_id_A, activity_result);

  {
    expect_cancel const _ (this, child_id_B);

    do_cancel (id);
  }

  {
    expect_canceled const _ (this, id);

    //! \todo There is an uncheckable(?) race here: rts_canceled may
    //! be called before do_canceled (second child)!

    do_canceled (child_id_B);
  }
}

BOOST_FIXTURE_TEST_CASE (child_failure_shall_fail_parent, daemon)
{
  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (1);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id;
  {
    expect_submit const _ (this, &child_id, activity_child);

    do_submit (id, activity_input);
  }

  const std::string message (rand() % 0xFE + 1, rand() % 0xFE + 1);

  {
    expect_failed const _ (this, id, message);

    do_failed (child_id, message);
  }
}

BOOST_FIXTURE_TEST_CASE
  (sibling_jobs_shall_be_canceled_on_child_failure, daemon)
{
  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (2);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id_A;
  we::layer::id_type child_id_B;

  {
    expect_submit const _A (this, &child_id_A, activity_child);
    expect_submit const _B (this, &child_id_B, activity_child);

    do_submit (id, activity_input);
  }

  const std::string message (rand() % 0xFE + 1, rand() % 0xFE + 1);

  {
    expect_cancel const _ (this, child_id_B);

    do_failed (child_id_A, message);
  }

  {
    expect_failed const _ (this, id, message);

    //! \todo There is an uncheckable(?) race here: rts_failed may
    //! be called before do_canceled (second child)!

    do_canceled (child_id_B);
  }
}

BOOST_FIXTURE_TEST_CASE
  (finished_shall_be_called_after_finished_N_childs, daemon)
{
  const std::size_t N (10);

  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (N);

  we::layer::id_type const id (generate_id());

  std::vector<we::layer::id_type> child_ids (N);

  {
    boost::ptr_vector<expect_submit> _;
    _.reserve (N);
    for (std::size_t i (0); i < N; ++i)
    {
      _.push_back (new expect_submit (this, &child_ids[i], activity_child));
    }

    do_submit (id, activity_input);
  }

  for (std::size_t i (0); i < N - 1; ++i)
  {
    do_finished (child_ids[i], activity_result);
  }

  {
    expect_finished const _ (this, id, activity_output);

    do_finished (child_ids.back(), activity_result);
  }
}

#ifdef NDEBUG
namespace
{
  void submit_fake ( std::vector<we::layer::id_type>* ids
                   , we::layer::id_type id
                   , we::type::activity_t
                   )
  {
    ids->push_back (id);
  }

  void finished_fake ( volatile bool* finished
                     , we::layer::id_type
                     , we::type::activity_t
                     )
  {
    *finished = true;
  }

  void cancel (we::layer::id_type){}
  void failed (we::layer::id_type, std::string){}
  void canceled (we::layer::id_type){}
  void discover (we::layer::id_type, we::layer::id_type){}
  void discovered (we::layer::id_type, sdpa::discovery_info_t){}

  boost::mutex generate_id_mutex;
  we::layer::id_type generate_id()
  {
    boost::mutex::scoped_lock const _ (generate_id_mutex);
    static unsigned long _cnt (0);
    return boost::lexical_cast<we::layer::id_type> (++_cnt);
  }
}

BOOST_AUTO_TEST_CASE
  (performance_finished_shall_be_called_after_finished_N_childs)
{
  const std::size_t num_activities (10);
  const std::size_t num_child_per_activity (250);

  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (num_child_per_activity);

  std::vector<we::layer::id_type> child_ids;
  child_ids.reserve (num_child_per_activity * num_activities);

  bool finished (false);

  std::mt19937 _random_engine;

  we::layer layer
    ( std::bind (&submit_fake, &child_ids, std::placeholders::_1, std::placeholders::_2)
    , std::bind (&cancel, std::placeholders::_1)
    , std::bind (&finished_fake, &finished, std::placeholders::_1, std::placeholders::_2)
    , &failed
    , &canceled
    , &discover
    , &discovered
    , &generate_id
    , _random_engine
    );

  double t (-fhg::util::now());

  for (std::size_t i (0); i < num_activities; ++i)
  {
    layer.submit (generate_id(), activity_input);
  }

  //! \todo Don't busy wait
  while (child_ids.size() != child_ids.capacity())
  {
    boost::this_thread::yield();
  }

  for (we::layer::id_type child_id : child_ids)
  {
    layer.finished (child_id, activity_result);
  }

  //! \todo Don't busy wait
  while (!finished)
  {
    boost::this_thread::yield();
  }

  t += fhg::util::now();

  BOOST_REQUIRE_LT (t, 1.0);
}
#endif


BOOST_FIXTURE_TEST_CASE
  (discovered_shall_be_called_after_discover_one_child, daemon)
{
  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (1);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id;
  {
    expect_submit _ (this, &child_id, activity_child);

    do_submit (id, activity_input);
  }

  const we::layer::id_type discover_id
    (std::string (rand() % 0xFE + 1, rand() % 0xFE + 1));
  {
    expect_discover _ (this, discover_id, child_id);

    do_discover (discover_id, id);
  }

  sdpa::discovery_info_t discover_result_child( child_id
                                                , sdpa::status::PENDING
                                                , sdpa::discovery_info_set_t());

  sdpa::discovery_info_set_t child_disc_set;
  child_disc_set.insert(discover_result_child);
  sdpa::discovery_info_t discover_result(id, boost::none, child_disc_set);

  {
    expect_discovered _ (this, discover_id, discover_result);

    do_discovered (discover_id, discover_result_child);
  }

  {
    expect_finished const _ (this, id, activity_output);

    do_finished (child_id, activity_result);
  }
}

BOOST_FIXTURE_TEST_CASE
  (discovered_shall_be_called_after_discover_two_childs, daemon)
{
  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (2);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id_A;
  we::layer::id_type child_id_B;
  {
    expect_submit _A (this, &child_id_A, activity_child);
    expect_submit _B (this, &child_id_B, activity_child);

    do_submit (id, activity_input);
  }

  const we::layer::id_type discover_id
    (std::string (rand() % 0xFE + 1, rand() % 0xFE + 1));
  {
    expect_discover _A (this, discover_id, child_id_A);
    expect_discover _B (this, discover_id, child_id_B);

    do_discover (discover_id, id);
  }

  using pnet::type::value::poke;

  sdpa::discovery_info_t discover_result_child_A( child_id_A
                                                  , sdpa::status::PENDING
                                                  , sdpa::discovery_info_set_t());

  sdpa::discovery_info_t discover_result_child_B( child_id_B
                                                  , sdpa::status::PENDING
                                                  , sdpa::discovery_info_set_t());

  sdpa::discovery_info_set_t child_disc_set;
  child_disc_set.insert(discover_result_child_A);
  child_disc_set.insert(discover_result_child_B);
  sdpa::discovery_info_t discover_result(id, boost::none, child_disc_set);

  do_discovered (discover_id, discover_result_child_A);

  {
    expect_discovered _ (this, discover_id, discover_result);

    //! \note There is a race here where layer may call rts_discovered
    //! before do_discovered, but this is checked by comparing the
    //! result: if it would finish discovering before the second child
    //! was discovered, there would be a child entry missing.

    do_discovered (discover_id, discover_result_child_B);
  }

  {
    expect_finished const _ (this, id, activity_output);

    do_finished (child_id_A, activity_result);
    do_finished (child_id_B, activity_result);
  }
}

namespace std
{
  template<> struct hash<we::type::requirement_t>
  {
    size_t operator()(we::type::requirement_t const& requirement) const
    {
      size_t seed (0);
      boost::hash_combine (seed, requirement.value());
      boost::hash_combine (seed, requirement.is_mandatory());
      return seed;
    }
  };
}

namespace
{
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

  class wfe_and_counter_of_submitted_requirements
  {
  public:
    wfe_and_counter_of_submitted_requirements (unsigned int expected_activities)
      : _expected_activities (expected_activities)
      , _received_requirements()
      , _random_extraction_engine()
      , _cnt (0)
      , _layer ( std::bind
               (&wfe_and_counter_of_submitted_requirements::submit, this, std::placeholders::_2)
               , std::bind (&disallow, "cancel")
               , std::bind (&disallow, "finished")
               , std::bind (&disallow, "failed")
               , std::bind (&disallow, "canceled")
               , std::bind (&disallow, "discover")
               , std::bind (&disallow, "discovered")
               , std::bind
                 (&wfe_and_counter_of_submitted_requirements::generate_id, this)
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
    std::unordered_map<we::type::requirement_t, unsigned int>
      _received_requirements;

  private:
    std::mt19937 _random_extraction_engine;

    boost::mutex _generate_id_mutex;
    unsigned long _cnt;
    we::layer::id_type generate_id()
    {
      boost::mutex::scoped_lock const _ (_generate_id_mutex);
      return boost::lexical_cast<we::layer::id_type> (++_cnt);
    }

  public:
    we::layer _layer;
  };
}

BOOST_AUTO_TEST_CASE (layer_properly_forwards_requirements)
{
  wfe_and_counter_of_submitted_requirements helper (30);

  const we::type::requirement_t req_A ("A", true);
  const we::type::requirement_t req_B ("B", true);

  helper._layer.submit
    ( fhg::util::random_string()
    , net_with_two_childs_that_require_capabilities (req_A, 20, req_B, 10)
    );
  helper.wait_all_submitted();

  BOOST_REQUIRE_EQUAL (helper._received_requirements.size(), 2);
  BOOST_REQUIRE_EQUAL (helper._received_requirements.at (req_A), 20);
  BOOST_REQUIRE_EQUAL (helper._received_requirements.at (req_B), 10);
}
