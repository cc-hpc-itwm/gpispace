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

#include <fhg/util/now.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <list>

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
    , layer ( boost::bind (&daemon::submit, this, _1, _2)
            , boost::bind (&daemon::cancel, this, _1)
            , boost::bind (&daemon::finished, this, _1, _2)
            , boost::bind (&daemon::failed, this, _1, _2, _3)
            , boost::bind (&daemon::canceled, this, _1)
            , boost::bind (&daemon::generate_id, this)
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
                       , we::mgmt::layer::id_type* id
        BOOST_PP_COMMA() we::mgmt::type::activity_t act
                       , _id (id)
        BOOST_PP_COMMA() _act (act)
                       , we::mgmt::layer::id_type* _id
                       ; we::mgmt::type::activity_t _act
                       , _act == act
                       );

  void submit
    (we::mgmt::layer::id_type id, we::mgmt::type::activity_t act)
  {
    INC_IN_PROGRESS (jobs_rts);

    std::list<expect_submit*>::iterator const e
      ( boost::find_if ( _to_submit
                       , boost::bind (&expect_submit::eq, _1, &id, act)
                       )
        );

    BOOST_REQUIRE (e != _to_submit.end());

    *((*e)->_id) = id;

    (*e)->happened();
    _to_submit.erase (e);
  }

  DECLARE_EXPECT_CLASS ( cancel
                       , we::mgmt::layer::id_type id
                       , _id (id)
                       , we::mgmt::layer::id_type _id
                       , _id == id
                       );

  void cancel (we::mgmt::layer::id_type id)
  {
    INC_IN_PROGRESS (replies);

    std::list<expect_cancel*>::iterator const e
      ( boost::find_if ( _to_cancel
                       , boost::bind (&expect_cancel::eq, _1, id)
                       )
        );

    BOOST_REQUIRE (e != _to_cancel.end());

    (*e)->happened();
    _to_cancel.erase (e);
  }


  DECLARE_EXPECT_CLASS ( finished
                       , we::mgmt::layer::id_type id
        BOOST_PP_COMMA() we::mgmt::type::activity_t act
                       , _id (id)
        BOOST_PP_COMMA() _act (act)
                       , we::mgmt::layer::id_type _id
                       ; we::mgmt::type::activity_t _act
                       , _id == id && _act == act
                       );

  void finished ( we::mgmt::layer::id_type id
                , we::mgmt::type::activity_t act
                )
  {
    std::list<expect_finished*>::iterator const e
      ( boost::find_if ( _to_finished
                       , boost::bind (&expect_finished::eq, _1, id, act)
                       )
      );

    BOOST_REQUIRE (e != _to_finished.end());

    (*e)->happened();
    _to_finished.erase (e);

    DEC_IN_PROGRESS (jobs_layer);
  }

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
                       , _id == id && _ec == ec && _message == message
                       );

  void failed ( we::mgmt::layer::id_type id
              , int ec
              , std::string message
              )
  {
    std::list<expect_failed*>::iterator const e
      ( boost::find_if ( _to_failed
                       , boost::bind (&expect_failed::eq, _1, id, ec, message)
                       )
      );

    BOOST_REQUIRE (e != _to_failed.end());

    (*e)->happened();
    _to_failed.erase (e);

    DEC_IN_PROGRESS (jobs_layer);
  }

  DECLARE_EXPECT_CLASS ( canceled
                       , we::mgmt::layer::id_type id
                       , _id (id)
                       , we::mgmt::layer::id_type _id
                       , _id == id
                       );

  void canceled (we::mgmt::layer::id_type id)
  {
    std::list<expect_canceled*>::iterator const e
      ( boost::find_if ( _to_canceled
                       , boost::bind (&expect_canceled::eq, _1, id)
                       )
      );

    BOOST_REQUIRE (e != _to_canceled.end());

    (*e)->happened();
    _to_canceled.erase (e);

    DEC_IN_PROGRESS (jobs_layer);
    DEC_IN_PROGRESS (replies);
  }

  void do_submit ( we::mgmt::layer::id_type id
                 , we::mgmt::type::activity_t act
                 )
  {
    INC_IN_PROGRESS (jobs_layer);

    layer.submit (id, act);
  }

  void do_finished ( we::mgmt::layer::id_type id
                   , we::mgmt::type::activity_t act
                   )
  {
    DEC_IN_PROGRESS (jobs_rts);

    layer.finished (id, act);
  }

  void do_cancel (we::mgmt::layer::id_type id)
  {
    INC_IN_PROGRESS (replies);

    layer.cancel (id);
  }

  void do_failed ( we::mgmt::layer::id_type id
                 , int ec
                 , std::string message
                 )
  {
    DEC_IN_PROGRESS (jobs_rts);

    layer.failed (id, ec, message);
  }

  void do_canceled (we::mgmt::layer::id_type id)
  {
    DEC_IN_PROGRESS (replies);
    DEC_IN_PROGRESS (jobs_rts);

    layer.canceled (id);
  }

  boost::mutex _generate_id_mutex;
  unsigned long _cnt;
  we::mgmt::layer::id_type generate_id()
  {
    boost::mutex::scoped_lock const _ (_generate_id_mutex);
    return boost::lexical_cast<we::mgmt::layer::id_type> (++_cnt);
  }

  boost::mt19937 _random_engine;
  we::mgmt::layer layer;
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
    , condition::type ("true")
    , true
    , we::type::property::type()
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

BOOST_FIXTURE_TEST_CASE (module_calls_should_be_submitted_to_rts, daemon)
{
  we::type::transition_t transition
    ( "module call"
    , we::type::module_call_t ("m", "f")
    , condition::type ("true")
    , true
    , we::type::property::type()
    );
  petri_net::port_id_type const port_id_in
    ( transition.add_port ( we::type::port_t ( "in"
                                             , we::type::PORT_IN
                                             , signature::CONTROL
                                             , we::type::property::type()
                                             )
                          )
    );
  petri_net::port_id_type const port_id_out
    ( transition.add_port ( we::type::port_t ( "out"
                                             , we::type::PORT_OUT
                                             , signature::CONTROL
                                             , we::type::property::type()
                                             )
                          )
    );

  we::mgmt::type::activity_t activity_output (transition);
  activity_output.add_output
    (transition.output_port_by_name ("out"), value::CONTROL);

  we::mgmt::type::activity_t activity_input (transition);
  activity_input.add_input
    (transition.input_port_by_name ("in"), value::CONTROL);

  //! \todo leaking implementation detail: maybe extract() should
  //! remove those connections to outside that were introduced by
  //! wrap()
  transition.add_connection (port_id_out, 0, we::type::property::type());
  transition.add_connection (1, port_id_in, we::type::property::type());
  we::mgmt::type::activity_t activity_child (transition);
  activity_child.add_input
    (transition.input_port_by_name ("in"), value::CONTROL);

  we::mgmt::type::activity_t activity_result (transition);
  activity_result.add_output
    (transition.output_port_by_name ("out"), value::CONTROL);

  we::mgmt::layer::id_type const id (generate_id());

  {
    expect_finished const _ (this, id, activity_output);

    we::mgmt::layer::id_type child_id;
    {
      expect_submit const _ (this, &child_id, activity_child);

      do_submit (id, activity_input);
    }

    do_finished (child_id, activity_result);
  }
}

namespace
{
  std::pair<we::type::transition_t, we::type::transition_t>
    finished_shall_be_called_after_finished_make_net ( bool put_on_input
                                                     , std::size_t token_count
                                                     )
  {
    we::type::transition_t transition
      ( "module call"
      , we::type::module_call_t ("m", "f")
      , condition::type ("true")
      , true
      , we::type::property::type()
      );
    petri_net::port_id_type const port_id_in
      ( transition.add_port ( we::type::port_t ( "in"
                                               , we::type::PORT_IN
                                               , signature::CONTROL
                                               , we::type::property::type()
                                               )
                            )
      );
    petri_net::port_id_type const port_id_out
      ( transition.add_port ( we::type::port_t ( "out"
                                               , we::type::PORT_OUT
                                               , signature::CONTROL
                                               , we::type::property::type()
                                               )
                            )
      );

    petri_net::net net;

    petri_net::place_id_type const place_id_in
      (net.add_place (place::type ("in", signature::CONTROL)));
    petri_net::place_id_type const place_id_out
      (net.add_place (place::type ("out", signature::CONTROL)));

    for (std::size_t i (0); i < token_count; ++i)
    {
      net.put_value (put_on_input ? place_id_in : place_id_out, value::CONTROL);
    }

    transition.add_connection (place_id_in, port_id_in, we::type::property::type());
    transition.add_connection (port_id_out, place_id_out, we::type::property::type());

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

BOOST_FIXTURE_TEST_CASE
  (finished_shall_be_called_after_finished_one_child, daemon)
{
  we::type::transition_t transition_in;
  we::type::transition_t transition_out;
  we::type::transition_t transition_child;
  boost::tie (transition_in, transition_child) =
    finished_shall_be_called_after_finished_make_net (true, 1);
  boost::tie (transition_out, boost::tuples::ignore) =
    finished_shall_be_called_after_finished_make_net (false, 1);

  we::mgmt::type::activity_t activity_input (transition_in);
  we::mgmt::type::activity_t activity_output (transition_out);

  we::mgmt::type::activity_t activity_child (transition_child);
  activity_child.add_input
    (transition_child.input_port_by_name ("in"), value::CONTROL);

  we::mgmt::type::activity_t activity_result (transition_child);
  activity_result.add_output
    (transition_child.output_port_by_name ("out"), value::CONTROL);

  we::mgmt::layer::id_type const id (generate_id());

  we::mgmt::layer::id_type child_id;

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
  we::type::transition_t transition_in;
  we::type::transition_t transition_out;
  we::type::transition_t transition_child;
  boost::tie (transition_in, transition_child) =
    finished_shall_be_called_after_finished_make_net (true, 2);
  boost::tie (transition_out, boost::tuples::ignore) =
    finished_shall_be_called_after_finished_make_net (false, 2);

  we::mgmt::type::activity_t activity_input (transition_in);
  we::mgmt::type::activity_t activity_output (transition_out);

  we::mgmt::type::activity_t activity_child (transition_child);
  activity_child.add_input
    (transition_child.input_port_by_name ("in"), value::CONTROL);

  we::mgmt::type::activity_t activity_result (transition_child);
  activity_result.add_output
    (transition_child.output_port_by_name ("out"), value::CONTROL);

  we::mgmt::layer::id_type const id (generate_id());

  we::mgmt::layer::id_type child_id_A;
  we::mgmt::layer::id_type child_id_B;

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
  (canceled_shall_be_called_after_cancel_one_child, daemon)
{
  we::type::transition_t transition_in;
  we::type::transition_t transition_out;
  we::type::transition_t transition_child;
  boost::tie (transition_in, transition_child) =
    finished_shall_be_called_after_finished_make_net (true, 1);
  boost::tie (transition_out, boost::tuples::ignore) =
    finished_shall_be_called_after_finished_make_net (false, 1);

  we::mgmt::type::activity_t activity_input (transition_in);
  we::mgmt::type::activity_t activity_output (transition_out);

  we::mgmt::type::activity_t activity_child (transition_child);
  activity_child.add_input
    (transition_child.input_port_by_name ("in"), value::CONTROL);

  we::mgmt::type::activity_t activity_result (transition_child);
  activity_result.add_output
    (transition_child.output_port_by_name ("out"), value::CONTROL);

  we::mgmt::layer::id_type const id (generate_id());

  we::mgmt::layer::id_type child_id;

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
  we::type::transition_t transition_in;
  we::type::transition_t transition_out;
  we::type::transition_t transition_child;
  boost::tie (transition_in, transition_child) =
    finished_shall_be_called_after_finished_make_net (true, 2);
  boost::tie (transition_out, boost::tuples::ignore) =
    finished_shall_be_called_after_finished_make_net (false, 2);

  we::mgmt::type::activity_t activity_input (transition_in);
  we::mgmt::type::activity_t activity_output (transition_out);

  we::mgmt::type::activity_t activity_child (transition_child);
  activity_child.add_input
    (transition_child.input_port_by_name ("in"), value::CONTROL);

  we::mgmt::type::activity_t activity_result (transition_child);
  activity_result.add_output
    (transition_child.output_port_by_name ("out"), value::CONTROL);

  we::mgmt::layer::id_type const id (generate_id());

  we::mgmt::layer::id_type child_id_A;
  we::mgmt::layer::id_type child_id_B;

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
  (sibling_jobs_shall_be_canceled_on_child_failure, daemon)
{
  we::type::transition_t transition_in;
  we::type::transition_t transition_out;
  we::type::transition_t transition_child;
  boost::tie (transition_in, transition_child) =
    finished_shall_be_called_after_finished_make_net (true, 2);
  boost::tie (transition_out, boost::tuples::ignore) =
    finished_shall_be_called_after_finished_make_net (false, 2);

  we::mgmt::type::activity_t activity_input (transition_in);
  we::mgmt::type::activity_t activity_output (transition_out);

  we::mgmt::type::activity_t activity_child (transition_child);
  activity_child.add_input
    (transition_child.input_port_by_name ("in"), value::CONTROL);

  we::mgmt::type::activity_t activity_result (transition_child);
  activity_result.add_output
    (transition_child.output_port_by_name ("out"), value::CONTROL);

  we::mgmt::layer::id_type const id (generate_id());

  we::mgmt::layer::id_type child_id_A;
  we::mgmt::layer::id_type child_id_B;

  {
    expect_submit const _A (this, &child_id_A, activity_child);
    expect_submit const _B (this, &child_id_B, activity_child);

    do_submit (id, activity_input);
  }

  const int ec (rand());
  const std::string message (rand() % 0xFE + 1, rand() % 0xFE + 1);

  {
    expect_cancel const _ (this, child_id_B);

    do_failed (child_id_A, ec, message);
  }

  {
    expect_failed const _ (this, id, ec, message);

    //! \todo There is an uncheckable(?) race here: rts_failed may
    //! be called before do_canceled (second child)!

    do_canceled (child_id_B);
  }
}

BOOST_FIXTURE_TEST_CASE
  (finished_shall_be_called_after_finished_N_childs, daemon)
{
  const std::size_t N (10);

  we::type::transition_t transition_in;
  we::type::transition_t transition_out;
  we::type::transition_t transition_child;
  boost::tie (transition_in, transition_child) =
    finished_shall_be_called_after_finished_make_net (true, N);
  boost::tie (transition_out, boost::tuples::ignore) =
    finished_shall_be_called_after_finished_make_net (false, N);

  we::mgmt::type::activity_t activity_input (transition_in);
  we::mgmt::type::activity_t activity_output (transition_out);

  we::mgmt::type::activity_t activity_child (transition_child);
  activity_child.add_input
    (transition_child.input_port_by_name ("in"), value::CONTROL);

  we::mgmt::type::activity_t activity_result (transition_child);
  activity_result.add_output
    (transition_child.output_port_by_name ("out"), value::CONTROL);

  we::mgmt::layer::id_type const id (generate_id());

  std::vector<we::mgmt::layer::id_type> child_ids (N);

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
  void submit_fake ( std::vector<we::mgmt::layer::id_type>* ids
                   , we::mgmt::layer::id_type id
                   , we::mgmt::type::activity_t
                   )
  {
    ids->push_back (id);
  }

  void finished_fake ( volatile bool* finished
                     , we::mgmt::layer::id_type id
                     , we::mgmt::type::activity_t
                     )
  {
    *finished = true;
  }

  void cancel (we::mgmt::layer::id_type){}
  void failed (we::mgmt::layer::id_type, int errorcode, std::string reason){}
  void canceled (we::mgmt::layer::id_type){}

  boost::mutex generate_id_mutex;
  we::mgmt::layer::id_type generate_id()
  {
    boost::mutex::scoped_lock const _ (generate_id_mutex);
    static unsigned long _cnt (0);
    return boost::lexical_cast<we::mgmt::layer::id_type> (++_cnt);
  }
}

BOOST_AUTO_TEST_CASE (performance_finished_shall_be_called_after_finished_N_childs)
{
  const std::size_t num_activities (10);
  const std::size_t num_child_per_activity (250);

  we::type::transition_t transition_in;
  we::type::transition_t transition_out;
  we::type::transition_t transition_child;
  boost::tie (transition_in, transition_child) =
    finished_shall_be_called_after_finished_make_net
      (true, num_child_per_activity);
  boost::tie (transition_out, boost::tuples::ignore) =
    finished_shall_be_called_after_finished_make_net
      (false, num_child_per_activity);

  we::mgmt::type::activity_t activity_input (transition_in);
  we::mgmt::type::activity_t activity_output (transition_out);

  we::mgmt::type::activity_t activity_child (transition_child);
  activity_child.add_input
    (transition_child.input_port_by_name ("in"), value::CONTROL);

  we::mgmt::type::activity_t activity_result (transition_child);
  activity_result.add_output
    (transition_child.output_port_by_name ("out"), value::CONTROL);

  std::vector<we::mgmt::layer::id_type> child_ids;
  child_ids.reserve (num_child_per_activity * num_activities);

  bool finished (false);

  boost::mt19937 _random_engine;

  we::mgmt::layer layer
    ( boost::bind (&submit_fake, &child_ids, _1, _2)
    , boost::bind (&cancel, _1)
    , boost::bind (&finished_fake, &finished, _1, _2)
    , boost::bind (&failed, _1, _2, _3)
    , boost::bind (&canceled, _1)
    , boost::bind (&generate_id)
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

  BOOST_FOREACH (we::mgmt::layer::id_type child_id, child_ids)
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
