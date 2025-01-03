// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <we/layer.hpp>
#include <we/test/operator_equal.hpp>
#include <we/type/Activity.hpp>
#include <we/type/Expression.hpp>
#include <we/type/ModuleCall.hpp>
#include <we/type/Transition.hpp>
#include <we/type/signature.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <we/test/layer.common.hpp>

#include <util-generic/serialization/exception.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/list.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/random/string.hpp>

#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <tuple>

namespace
{
  bool compare_workflow_response_result
    ( std::variant<std::exception_ptr, pnet::type::value::value_type> const& l
    , std::variant<std::exception_ptr, pnet::type::value::value_type> const& r
    )
  {
    struct visitor
    {
      bool operator() (std::exception_ptr const&, pnet::type::value::value_type const&) const { return false; }
      bool operator() (pnet::type::value::value_type const&, std::exception_ptr const&) const { return false; }

      bool operator() ( pnet::type::value::value_type const& lhs
                      , pnet::type::value::value_type const& rhs
                      ) const
      {
        return lhs == rhs;
      }

      bool operator() ( std::exception_ptr const& lhs
                      , std::exception_ptr const& rhs
                      ) const
      {
        //! \note hacky-di-hack
        return fhg::util::serialization::exception::serialize (lhs)
          == fhg::util::serialization::exception::serialize (rhs);
      }
    };
    return std::visit (visitor{}, l, r);
  }
}

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
      std::unique_lock<std::mutex> lock (_happened_mutex);              \
      _happened_condition.wait (lock, [this]() { return _happened; });  \
    }                                                                   \
    expect_ ## NAME (expect_ ## NAME const&) = delete;                  \
    expect_ ## NAME& operator= (expect_ ## NAME const&) = delete;       \
    expect_ ## NAME (expect_ ## NAME &&) = delete;                      \
    expect_ ## NAME& operator= (expect_ ## NAME &&) = delete;           \
                                                                        \
    void happened()                                                     \
    {                                                                   \
      std::lock_guard<std::mutex> const lock (_happened_mutex);         \
      _happened = true;                                                 \
      _happened_condition.notify_one();                                 \
    }                                                                   \
    bool eq (CTOR_ARGUMENTS) const                                      \
    {                                                                   \
      return EQ_IMPL;                                                   \
    }                                                                   \
    bool _happened;                                                     \
    std::mutex _happened_mutex;                                         \
    std::condition_variable _happened_condition;                        \
                                                                        \
    MEMBER_VARIABLES;                                                   \
  };                                                                    \
                                                                        \
  std::list<expect_ ## NAME*> _to_ ## NAME

struct daemon
{
  daemon()
    : layer ( std::bind (&daemon::submit, this, std::placeholders::_1, std::placeholders::_2)
            , std::bind (&daemon::cancel, this, std::placeholders::_1)
            , std::bind (&daemon::finished, this, std::placeholders::_1, std::placeholders::_2)
            , std::bind (&daemon::failed, this, std::placeholders::_1, std::placeholders::_2)
            , std::bind (&daemon::canceled, this, std::placeholders::_1)
            , std::bind (&daemon::token_put, this, std::placeholders::_1)
            , std::bind (&daemon::workflow_response, this, std::placeholders::_1, std::placeholders::_2)
            , std::bind (&daemon::generate_id, this)
            , _random_engine
            )
  {}
  ~daemon()
  {
    std::unique_lock<std::mutex> lock (_in_progress_mutex);

    _in_progress_condition.wait
      ( lock
      , [this]()
        {
          return _in_progress_jobs_rts
               + _in_progress_jobs_layer
               + _in_progress_replies
               + _in_progress_cancels.size()
               == 0
            ;
        }
      );
  }
  daemon (daemon const&) = delete;
  daemon& operator= (daemon const&) = delete;
  daemon (daemon&&) = delete;
  daemon& operator= (daemon&&) = delete;

#define INC_IN_PROGRESS(COUNTER)                                \
  do                                                            \
  {                                                             \
    std::lock_guard<std::mutex> const _ (_in_progress_mutex);   \
    ++_in_progress_ ## COUNTER;                                 \
  } while (false)

#define DEC_IN_PROGRESS(COUNTER)                                \
  {                                                             \
    std::lock_guard<std::mutex> const _ (_in_progress_mutex);   \
    --_in_progress_ ## COUNTER;                                 \
  }                                                             \
  _in_progress_condition.notify_one()

  std::unordered_set<we::layer::id_type> _in_progress_cancels;

#define ADD_CANCEL_IN_PROGRESS(ID)                              \
  do                                                            \
  {                                                             \
    std::lock_guard<std::mutex> const _ (_in_progress_mutex);   \
    _in_progress_cancels.emplace (ID);                          \
  } while (false)

#define REMOVE_CANCEL_IN_PROGRESS(ID)                           \
  do                                                            \
  {                                                             \
    std::lock_guard<std::mutex> const _ (_in_progress_mutex);   \
    _in_progress_cancels.erase (ID);                            \
  } while (false)

  DECLARE_EXPECT_CLASS ( submit
                       , we::layer::id_type* id
        BOOST_PP_COMMA() we::type::Activity act
                       , _id (id)
        BOOST_PP_COMMA() _act (act)
                       , we::layer::id_type* _id
                       ; we::type::Activity _act
                       , id != nullptr && _act == act
                       );

  void submit
    (we::layer::id_type id, we::type::Activity act)
  {
    INC_IN_PROGRESS (jobs_rts);

    std::list<expect_submit*>::iterator const e
      ( ::boost::find_if ( _to_submit
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
    ADD_CANCEL_IN_PROGRESS (id);

    std::list<expect_cancel*>::iterator const e
      ( ::boost::find_if ( _to_cancel
                       , std::bind (&expect_cancel::eq, std::placeholders::_1, id)
                       )
        );

    BOOST_REQUIRE (e != _to_cancel.end());

    (*e)->happened();
    _to_cancel.erase (e);
  }


  DECLARE_EXPECT_CLASS ( finished
                       , we::layer::id_type id
        BOOST_PP_COMMA() we::type::Activity act
                       , _id (id)
        BOOST_PP_COMMA() _act (act)
                       , we::layer::id_type _id
                       ; we::type::Activity _act
                       , _id == id && _act == act
                       );

  void finished ( we::layer::id_type id
                , we::type::Activity act
                )
  {
    std::list<expect_finished*>::iterator const e
      ( ::boost::find_if ( _to_finished
                       , std::bind (&expect_finished::eq, std::placeholders::_1, id, act)
                       )
      );

    BOOST_REQUIRE (e != _to_finished.end());

    (*e)->happened();
    _to_finished.erase (e);

    DEC_IN_PROGRESS (jobs_layer);
    REMOVE_CANCEL_IN_PROGRESS (id);
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
      ( ::boost::find_if ( _to_failed
                       , std::bind (&expect_failed::eq, std::placeholders::_1, id, message)
                       )
      );

    BOOST_REQUIRE (e != _to_failed.end());

    (*e)->happened();
    _to_failed.erase (e);

    DEC_IN_PROGRESS (jobs_layer);
    REMOVE_CANCEL_IN_PROGRESS (id);
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
      ( ::boost::find_if ( _to_canceled
                       , std::bind (&expect_canceled::eq, std::placeholders::_1, id)
                       )
      );

    BOOST_REQUIRE (e != _to_canceled.end());

    (*e)->happened();
    _to_canceled.erase (e);

    DEC_IN_PROGRESS (jobs_layer);
    REMOVE_CANCEL_IN_PROGRESS (id);
  }

  DECLARE_EXPECT_CLASS ( token_put
                       , std::string put_token_id
                       , _put_token_id (put_token_id)
                       , we::layer::id_type _put_token_id
                       , _put_token_id == put_token_id
                       );

  void token_put (std::string put_token_id)
  {
    std::list<expect_token_put*>::iterator const e
      ( ::boost::find_if ( _to_token_put
                       , std::bind (&expect_token_put::eq, std::placeholders::_1, put_token_id)
                       )
      );

    BOOST_REQUIRE (e != _to_token_put.end());

    (*e)->happened();
    _to_token_put.erase (e);

    DEC_IN_PROGRESS (replies);
  }

  DECLARE_EXPECT_CLASS ( workflow_response
                       , std::string workflow_response_id
        BOOST_PP_COMMA() std::variant<std::exception_ptr BOOST_PP_COMMA() pnet::type::value::value_type> result
                       , _workflow_response_id (workflow_response_id)
        BOOST_PP_COMMA() _result (result)
                       , we::layer::id_type _workflow_response_id
                       ; std::variant<std::exception_ptr BOOST_PP_COMMA() pnet::type::value::value_type> _result
                       , _workflow_response_id == workflow_response_id
                       && compare_workflow_response_result (_result, result)
                       );

  void workflow_response ( std::string workflow_response_id
                         , std::variant<std::exception_ptr, pnet::type::value::value_type> result
                         )
  {
    std::list<expect_workflow_response*>::iterator const e
      ( ::boost::find_if ( _to_workflow_response
                       , std::bind (&expect_workflow_response::eq, std::placeholders::_1, workflow_response_id, result)
                       )
      );

    BOOST_REQUIRE (e != _to_workflow_response.end());

    (*e)->happened();
    _to_workflow_response.erase (e);

    DEC_IN_PROGRESS (replies);
  }

  void do_submit ( we::layer::id_type id
                 , we::type::Activity act
                 )
  {
    INC_IN_PROGRESS (jobs_layer);

    layer.submit (id, act);
  }

  void do_finished ( we::layer::id_type id
                   , we::type::Activity act
                   )
  {
    DEC_IN_PROGRESS (jobs_rts);
    REMOVE_CANCEL_IN_PROGRESS (id);

    layer.finished (id, act);
  }

  void do_cancel (we::layer::id_type id)
  {
    ADD_CANCEL_IN_PROGRESS (id);

    layer.cancel (id);
  }

  void do_failed (we::layer::id_type id, std::string message)
  {
    DEC_IN_PROGRESS (jobs_rts);
    REMOVE_CANCEL_IN_PROGRESS (id);

    layer.failed (id, message);
  }

  void do_canceled (we::layer::id_type id)
  {
    DEC_IN_PROGRESS (jobs_rts);
    REMOVE_CANCEL_IN_PROGRESS (id);

    layer.canceled (id);
  }

  void do_put_token ( we::layer::id_type id
                    , std::string put_token_id
                    , std::string place_name
                    , pnet::type::value::value_type value
                    )
  {
    INC_IN_PROGRESS (replies);

    layer.put_token (id, put_token_id, place_name, value);
  }

  void do_workflow_response ( we::layer::id_type id
                            , std::string workflow_response_id
                            , std::string place_name
                            , pnet::type::value::value_type value
                            )
  {
    INC_IN_PROGRESS (replies);

    layer.request_workflow_response
      (id, workflow_response_id, place_name, value);
  }

  std::mutex _generate_id_mutex;
  unsigned long _cnt{};
  we::layer::id_type generate_id()
  {
    std::lock_guard<std::mutex> const _ (_generate_id_mutex);
    return std::to_string (++_cnt);
  }

  std::mt19937 _random_engine;
  mutable std::mutex _in_progress_mutex;
  we::layer layer;
  std::condition_variable _in_progress_condition;
  unsigned long _in_progress_jobs_rts{};
  unsigned long _in_progress_jobs_layer{};
  unsigned long _in_progress_replies{};
};

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE (expressions_shall_not_be_sumitted_to_rts, daemon)
{
  we::type::Transition transition
    ( "expression"
    , we::type::Expression ("${out} := ${in} + 1L")
    , ::boost::none
    , we::type::property::type()
    , we::priority_type()
    , ::boost::optional<we::type::eureka_id_type>{}
    , std::list<we::type::Preference>{}
    );
  transition.add_port
    (we::type::Port ( "in"
                      , we::type::port::direction::In{}
                      , signature::LONG
                      , we::type::property::type()
                      )
    );
  transition.add_port
    (we::type::Port ( "out"
                      , we::type::port::direction::Out{}
                      , signature::LONG
                      , we::type::property::type()
                      )
    );

  we::type::Activity activity (transition);
  activity.add_input ("in", pnet::type::value::read ("1L"));

  we::layer::id_type const id (generate_id());

  {
    we::type::Activity activity_expected (transition);
    activity_expected.add_output_TESTING_ONLY
      ( "out"
      , pnet::type::value::read ("2L")
      );

    expect_finished const _ (this, id, activity_expected);

    do_submit (id, activity);
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE (module_calls_should_be_submitted_to_rts, daemon)
{
  we::type::Transition transition
    ( "module call"
    , we::type::ModuleCall
      ( "m"
      , "f"
      , std::unordered_map<std::string, we::type::MemoryBufferInfo>()
      , std::list<we::type::memory_transfer>()
      , std::list<we::type::memory_transfer>()
      , true
      , true
      )
    , ::boost::none
    , we::type::property::type()
    , we::priority_type()
    , ::boost::optional<we::type::eureka_id_type>{}
    , std::list<we::type::Preference>{}
    );
  transition.add_port ( we::type::Port ( "in"
                                         , we::type::port::direction::In{}
                                         , signature::CONTROL
                                         , we::type::property::type()
                                         )
                      );
  transition.add_port ( we::type::Port ( "out"
                                         , we::type::port::direction::Out{}
                                         , signature::CONTROL
                                         , we::type::property::type()
                                         )
                      );

  we::type::Activity activity_output (transition);
  activity_output.add_output_TESTING_ONLY ("out", value::CONTROL);

  we::type::Activity activity_input (transition);
  activity_input.add_input ("in", value::CONTROL);

  we::type::Activity activity_child (transition);
  activity_child.add_input ("in", value::CONTROL);

  we::type::Activity activity_result
    (we::type::TESTING_ONLY{}, transition, we::transition_id_type (0));
  activity_result.add_output_TESTING_ONLY ("out", value::CONTROL);

  we::layer::id_type const id (generate_id());

  {
    expect_finished const _finished (this, id, activity_output);

    we::layer::id_type child_id;
    {
      expect_submit const _submit (this, &child_id, activity_child);

      do_submit (id, activity_input);
    }

    do_finished (child_id, activity_result);
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (finished_shall_be_called_after_finished_one_child, daemon)
{
  we::type::Activity activity_input;
  we::type::Activity activity_output;
  we::type::Activity activity_child;
  we::type::Activity activity_result;
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

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (finished_shall_be_called_after_finished_two_childs, daemon)
{
  we::type::Activity activity_input;
  we::type::Activity activity_output;
  we::type::Activity activity_child;
  we::type::Activity activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (2);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id_a;
  we::layer::id_type child_id_b;

  {
    expect_submit const _a (this, &child_id_a, activity_child);
    expect_submit const _b (this, &child_id_b, activity_child);

    do_submit (id, activity_input);
  }

  do_finished (child_id_a, activity_result);

  {
    expect_finished const _ (this, id, activity_output);

    //! \note There is a race here where layer may call rts_finished
    //! before do_finished, but this is checked by comparing the
    //! output activity: if it would finish before the second child
    //! finishing, there would be a token missing.

    do_finished (child_id_b, activity_result);
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (two_sequential_jobs_shall_be_properly_handled, daemon)
{
  we::type::Activity activity_input;
  we::type::Activity activity_output;
  we::type::Activity activity_child;
  we::type::Activity activity_result;
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

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (two_interleaving_jobs_shall_be_properly_handled, daemon)
{
  we::type::Activity activity_input;
  we::type::Activity activity_output;
  we::type::Activity activity_child;
  we::type::Activity activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (2);

  we::layer::id_type const id_0 (generate_id());
  we::layer::id_type const id_1 (generate_id());

  we::layer::id_type child_id_0_0;
  we::layer::id_type child_id_0_1;
  we::layer::id_type child_id_1_0;
  we::layer::id_type child_id_1_1;

  {
    expect_submit const _s0 (this, &child_id_0_0, activity_child);
    expect_submit const _s1 (this, &child_id_0_1, activity_child);

    do_submit (id_0, activity_input);
  }

  {
    expect_submit const _s0 (this, &child_id_1_0, activity_child);
    expect_submit const _s1 (this, &child_id_1_1, activity_child);

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

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (canceled_shall_be_called_after_cancel_one_child, daemon)
{
  we::type::Activity activity_input;
  we::type::Activity activity_output;
  we::type::Activity activity_child;
  we::type::Activity activity_result;
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

namespace
{
  struct cancel_test_daemon : public daemon
  {
    enum method
    {
      canceled
    , failed
    , finished
    };
    static std::array<method, 3> methods;

    void test_routine (method first_child, method second_child)
    {
      we::type::Activity activity_input;
      we::type::Activity activity_output;
      we::type::Activity activity_child;
      we::type::Activity activity_result;
      std::tie (activity_input, activity_output, activity_child, activity_result)
        = activity_with_child (2);

      we::layer::id_type const id (generate_id());

      we::layer::id_type child_id_a;
      we::layer::id_type child_id_b;

      {
        expect_submit const _a (this, &child_id_a, activity_child);
        expect_submit const _b (this, &child_id_b, activity_child);

        do_submit (id, activity_input);
      }

      {
        expect_cancel const _a (this, child_id_a);
        expect_cancel const _b (this, child_id_b);

        do_cancel (id);
      }

      auto&& do_method
        ( [&] (we::layer::id_type const& id_, method how)
          {
            switch (how)
            {
            case canceled: do_canceled (id_); break;
            case failed: do_failed (id_, "interleaved with cancel"); break;
            case finished: do_finished (id_, activity_result); break;
            }
          }
        );

      do_method (child_id_a, first_child);

      {
        expect_canceled const _ (this, id);

        //! \todo There is an uncheckable(?) race here: rts_canceled may
        //! be called before do_canceled (second child)!


        do_method (child_id_b, second_child);
      }
    }
  };
  std::array<cancel_test_daemon::method, 3> cancel_test_daemon::methods
    { { cancel_test_daemon::canceled
      , cancel_test_daemon::failed
      , cancel_test_daemon::finished
      }
    };
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (canceled_shall_be_called_after_cancel_two_childs, cancel_test_daemon)
{
  test_routine (canceled, canceled);
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_DATA_TEST_CASE
  ( any_terminiation_during_canceling_is_equivalent_to_canceled
  , ::boost::unit_test::data::make (cancel_test_daemon::methods)
  * ::boost::unit_test::data::make (cancel_test_daemon::methods)
  , first_child
  , second_child
  )
{
  cancel_test_daemon().test_routine (first_child, second_child);
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  ( canceled_shall_be_called_after_cancel_two_childs_with_one_child_finished
  , daemon
  )
{
  we::type::Activity activity_input;
  we::type::Activity activity_output;
  we::type::Activity activity_child;
  we::type::Activity activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (2);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id_a;
  we::layer::id_type child_id_b;

  {
    expect_submit const _a (this, &child_id_a, activity_child);
    expect_submit const _b (this, &child_id_b, activity_child);

    do_submit (id, activity_input);
  }

  do_finished (child_id_a, activity_result);

  {
    expect_cancel const _ (this, child_id_b);

    do_cancel (id);
  }

  {
    expect_canceled const _ (this, id);

    //! \todo There is an uncheckable(?) race here: rts_canceled may
    //! be called before do_canceled (second child)!

    do_canceled (child_id_b);
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE (child_failure_shall_fail_parent, daemon)
{
  we::type::Activity activity_input;
  we::type::Activity activity_output;
  we::type::Activity activity_child;
  we::type::Activity activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (1);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id;
  {
    expect_submit const _ (this, &child_id, activity_child);

    do_submit (id, activity_input);
  }

  const std::string message
    (fhg::util::testing::random_string_without_zero());

  {
    expect_failed const _ (this, id, message);

    do_failed (child_id, message);
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (sibling_jobs_shall_be_canceled_on_child_failure, daemon)
{
  we::type::Activity activity_input;
  we::type::Activity activity_output;
  we::type::Activity activity_child;
  we::type::Activity activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (2);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id_a;
  we::layer::id_type child_id_b;

  {
    expect_submit const _a (this, &child_id_a, activity_child);
    expect_submit const _b (this, &child_id_b, activity_child);

    do_submit (id, activity_input);
  }

  const std::string message
    (fhg::util::testing::random_string_without_zero());

  {
    expect_cancel const _ (this, child_id_b);

    do_failed (child_id_a, message);
  }

  {
    expect_failed const _ (this, id, message);

    //! \todo There is an uncheckable(?) race here: rts_failed may
    //! be called before do_canceled (second child)!

    do_canceled (child_id_b);
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (sibling_jobs_shall_be_canceled_on_child_failure_and_any_termination_is_okay, daemon)
{
  we::type::Activity activity_input;
  we::type::Activity activity_output;
  we::type::Activity activity_child;
  we::type::Activity activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (3);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id_a;
  we::layer::id_type child_id_b;
  we::layer::id_type child_id_c;

  {
    expect_submit const _a (this, &child_id_a, activity_child);
    expect_submit const _b (this, &child_id_b, activity_child);
    expect_submit const _c (this, &child_id_c, activity_child);

    do_submit (id, activity_input);
  }

  const std::string message
    (fhg::util::testing::random_string_without_zero());

  {
    expect_cancel const _b (this, child_id_b);
    expect_cancel const _c (this, child_id_c);

    do_failed (child_id_a, message);
  }

  {
    expect_failed const _ (this, id, message);

    //! \todo There is an uncheckable(?) race here: rts_failed may
    //! be called before do_* (second and third child)!

    do_finished (child_id_b, activity_result);
    do_failed (child_id_c, message);
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (finished_shall_be_called_after_finished_N_childs, daemon)
{
  const std::size_t N (10);

  we::type::Activity activity_input;
  we::type::Activity activity_output;
  we::type::Activity activity_child;
  we::type::Activity activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (N);

  we::layer::id_type const id (generate_id());

  std::vector<we::layer::id_type> child_ids (N);

  {
    std::list<std::unique_ptr<expect_submit>> expected_submissions;
    for (std::size_t i (0); i < N; ++i)
    {
      expected_submissions.emplace_back
        (std::make_unique<expect_submit> (this, &child_ids[i], activity_child));
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

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE (layer_properly_puts_token, daemon)
{
  we::type::Activity activity_input;
  we::type::Activity activity_output;
  we::type::Activity activity_child;
  we::type::Activity activity_result;
  std::tie (activity_input, std::ignore, activity_child, activity_result)
    = activity_with_child (1);
  std::tie (std::ignore, activity_output, std::ignore, std::ignore)
    = activity_with_child (2);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id_a;
  we::layer::id_type child_id_b;

  {
    expect_submit const _ (this, &child_id_a, activity_child);

    do_submit (id, activity_input);
  }

  {
    std::string const put_token_id (fhg::util::testing::random_string());
    expect_token_put const token_put (this, put_token_id);
    //! \note Race: can't ensure that child_id_b is triggered by putting token
    expect_submit const submitted (this, &child_id_b, activity_child);

    do_put_token (id, put_token_id, "in", value::CONTROL);
  }

  {
    //! \note Race: can't ensure that both finishes are required
    expect_finished const _ (this, id, activity_output);

    do_finished (child_id_a, activity_result);
    do_finished (child_id_b, activity_result);
  }
}

namespace
{
  std::tuple< we::type::Transition
            , we::type::Transition
            , we::transition_id_type
            >
    wfr_net_with_childs (bool put_on_input)
  {
    /* |> in -> [ trans_a ] -> mid -> [ trans_b ] -> out >|
                        *> request -> [         ] >*
    */

    pnet::type::signature::structured_type const request_t
      ( std::make_pair
          ( "request_t"
          , pnet::type::signature::structure_type
              ( { std::pair<std::string, std::string>
                    ("value", "long")
                , std::pair<std::string, std::string>
                    ("response_id", "string")
                }
              )
          )
      );

    we::type::net_type net;

    we::place_id_type const place_id_in
      (net.add_place (place::type ("in", signature::CONTROL, true, we::type::property::type{})));
    we::place_id_type const place_id_mid
      (net.add_place (place::type ("mid", signature::CONTROL, ::boost::none, we::type::property::type{})));
    we::place_id_type const place_id_out
      (net.add_place (place::type ("out", signature::CONTROL, ::boost::none, we::type::property::type{})));
    we::place_id_type const place_id_request
      (net.add_place (place::type ("request", request_t, true, we::type::property::type{})));


    we::type::Transition trans_a
      ( "trans_a"
      , we::type::ModuleCall
        ( "m"
        , "f"
        , std::unordered_map<std::string, we::type::MemoryBufferInfo>()
        , std::list<we::type::memory_transfer>()
        , std::list<we::type::memory_transfer>()
        , true
        , true
        )
      , ::boost::none
      , we::type::property::type()
      , we::priority_type()
      , ::boost::optional<we::type::eureka_id_type>{}
      , std::list<we::type::Preference>{}
      );
    we::port_id_type const trans_a_port_id_in
      ( trans_a.add_port ( we::type::Port ( "in"
                                            , we::type::port::direction::In{}
                                            , signature::CONTROL
                                            , we::type::property::type()
                                            )
                         )
      );
    we::port_id_type const trans_a_port_id_out
      ( trans_a.add_port ( we::type::Port ( "out"
                                            , we::type::port::direction::Out{}
                                            , signature::CONTROL
                                            , we::type::property::type()
                                            )
                         )
      );

    we::type::Transition trans_b
      ( "trans_b"
      , we::type::Expression
          ("${response} := ${request.value} + 1L; ${out} := ${in};")
      , ::boost::none
      , we::type::property::type()
      , we::priority_type()
      , ::boost::optional<we::type::eureka_id_type>{}
      , std::list<we::type::Preference>{}
      );
    we::port_id_type const trans_b_port_id_in
      ( trans_b.add_port ( we::type::Port ( "in"
                                            , we::type::port::direction::In{}
                                            , signature::CONTROL
                                            , we::type::property::type()
                                            )
                         )
      );
    we::port_id_type const trans_b_port_id_out
      ( trans_b.add_port ( we::type::Port ( "out"
                                            , we::type::port::direction::Out{}
                                            , signature::CONTROL
                                            , we::type::property::type()
                                            )
                         )
      );
    we::port_id_type const trans_b_port_id_request
      ( trans_b.add_port ( we::type::Port ( "request"
                                            , we::type::port::direction::In{}
                                            , request_t
                                            , we::type::property::type()
                                            )
                         )
      );
    we::port_id_type const trans_b_port_id_response
      ( trans_b.add_port ( we::type::Port ( "response"
                                            , we::type::port::direction::Out{}
                                            , signature::LONG
                                            , we::type::property::type()
                                            )
                         )
      );

    we::transition_id_type const trans_a_id
      (net.add_transition (trans_a));
    we::transition_id_type const trans_b_id
      (net.add_transition (trans_b));

      net.put_value (put_on_input ? place_id_in : place_id_out, value::CONTROL);

    {
      we::type::property::type empty;

      net.add_connection
        (we::edge::PT{}, trans_a_id, place_id_in, trans_a_port_id_in, empty);
      net.add_connection
        (we::edge::TP{}, trans_a_id, place_id_mid, trans_a_port_id_out, empty);

      net.add_connection
        (we::edge::PT{}, trans_b_id, place_id_mid, trans_b_port_id_in, empty);
      net.add_connection
        (we::edge::PT{}, trans_b_id, place_id_request, trans_b_port_id_request, empty);
      net.add_response
        (trans_b_id, trans_b_port_id_response, "request", empty);
      net.add_connection
        (we::edge::TP{}, trans_b_id, place_id_out, trans_b_port_id_out, empty);
    }

    return std::make_tuple
      ( we::type::Transition ( "net"
                               , net
                               , ::boost::none
                               , we::type::property::type()
                               , we::priority_type()
                               , ::boost::optional<we::type::eureka_id_type>{}
                               , std::list<we::type::Preference>{}
                               )
      , trans_a
      , trans_a_id
      );
  }

  std::tuple< we::type::Activity
            , we::type::Activity
            , we::type::Activity
            , we::type::Activity
            >
    wfr_activity_with_child()
  {
    we::transition_id_type transition_id_child;
    we::type::Transition transition_in;
    we::type::Transition transition_out;
    we::type::Transition transition_child;
    std::tie (transition_in, transition_child, transition_id_child) =
      wfr_net_with_childs (true);
    std::tie (transition_out, std::ignore, std::ignore) =
      wfr_net_with_childs (false);

    we::type::Activity activity_input (transition_in);
    we::type::Activity activity_output (transition_out);

    we::type::Activity activity_child (transition_child);
    activity_child.add_input ("in", value::CONTROL);

    we::type::Activity activity_result
      (we::type::TESTING_ONLY{}, transition_child, transition_id_child);
    activity_result.add_output_TESTING_ONLY ("out", value::CONTROL);

    return std::make_tuple
      (activity_input, activity_output, activity_child, activity_result);
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE (workflow_response_works, daemon)
{
  we::type::Activity activity_input;
  we::type::Activity activity_output;
  we::type::Activity activity_child;
  we::type::Activity activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = wfr_activity_with_child();

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id_a;

  {
    expect_submit const _ (this, &child_id_a, activity_child);

    do_submit (id, activity_input);
  }

  {
    std::string const workflow_response_id
      (fhg::util::testing::random_string());
    expect_workflow_response const workflow_response
      ( this
      , workflow_response_id
      , std::make_exception_ptr
          ( std::invalid_argument
              ( R"(put_token ("NONEXISTING_PLACE", Struct [value := 0L, response_id := ")"
              + workflow_response_id + "\"]): place not found"
              )
          )
      );

    do_workflow_response (id, workflow_response_id, "NONEXISTING_PLACE", 0L);
  }

  {
    std::string const workflow_response_id
      (fhg::util::testing::random_string());
    expect_workflow_response const workflow_response
      ( this
      , workflow_response_id
      , std::make_exception_ptr
          ( std::invalid_argument
              ( R"(put_token ("mid", Struct [value := 0L, response_id := ")"
              + workflow_response_id + R"("]): place not marked with attribute put_token="true")"
              )
          )
      );

    do_workflow_response (id, workflow_response_id, "mid", 0L);
  }

  {
    std::string const workflow_response_id
      (fhg::util::testing::random_string());
    expect_workflow_response const workflow_response
      ( this
      , workflow_response_id
      , std::make_exception_ptr
          ( pnet::exception::type_mismatch
              (signature::LONG, value::CONTROL, {"request", "value"})
          )
      );

    do_workflow_response (id, workflow_response_id, "request", value::CONTROL);
  }

  long const value {fhg::util::testing::random<long>()()};

  std::string const workflow_response_id
    (fhg::util::testing::random_string());
  do_workflow_response (id, workflow_response_id, "request", value);

  {
    expect_finished const finished (this, id, activity_output);

    expect_workflow_response const workflow_response
      (this, workflow_response_id, value + 1);

    do_finished (child_id_a, activity_result);
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE (workflow_response_fails_when_workflow_fails, daemon)
{
  we::type::Activity activity_input;
  we::type::Activity activity_child;
  std::tie (activity_input, std::ignore, activity_child, std::ignore)
    = wfr_activity_with_child();

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id;

  {
    expect_submit const _ (this, &child_id, activity_child);

    do_submit (id, activity_input);
  }

  std::string const workflow_response_id
    (fhg::util::testing::random_string());
  do_workflow_response (id, workflow_response_id, "request", 0L);

  {
    std::string const fail_reason (fhg::util::testing::random_string());
    expect_failed const failed (this, id, fail_reason);

    expect_workflow_response const workflow_response
      ( this
      , workflow_response_id
      , std::make_exception_ptr (std::runtime_error (fail_reason))
      );

    do_failed (child_id, fail_reason);
  }
}

namespace std
{
  template<> struct hash<we::type::Requirement>
  {
    size_t operator()(we::type::Requirement const& requirement) const
    {
      size_t seed (0);
      ::boost::hash_combine (seed, requirement.value());
      return seed;
    }
  };
}

namespace
{
  we::place_id_type add_transition_with_requirement_and_input_place
    (we::type::net_type& net, we::type::Requirement const& requirement)
  {
    we::type::Transition transition
      ( fhg::util::testing::random_string()
      , we::type::ModuleCall ( fhg::util::testing::random_string()
                                , fhg::util::testing::random_string()
                                , std::unordered_map< std::string
                                                    , we::type::MemoryBufferInfo
                                                    >()
                                , std::list<we::type::memory_transfer>()
                                , std::list<we::type::memory_transfer>()
                                , true
                                , true
                                )
      , ::boost::none
      , we::type::property::type()
      , we::priority_type()
      , ::boost::optional<we::type::eureka_id_type>{}
      , std::list<we::type::Preference>{}
      );
    transition.add_requirement (requirement);

    const std::string port_name (fhg::util::testing::random_string());
    we::port_id_type const port_id
      ( transition.add_port ( we::type::Port ( port_name
                                               , we::type::port::direction::In{}
                                               , std::string ("control")
                                               , we::type::property::type()
                                               )
                            )
      );

    we::transition_id_type const transition_id
      (net.add_transition (transition));

    we::place_id_type const place_id
      (net.add_place (place::type (port_name, std::string ("control"), ::boost::none, we::type::property::type{})));

    net.add_connection ( we::edge::PT{}
                       , transition_id
                       , place_id
                       , port_id
                       , we::type::property::type()
                       );

    return place_id;
  }

  we::type::Activity net_with_two_childs_that_require_capabilities
    ( we::type::Requirement const& capability_a
    , std::size_t num_worker_with_capability_a
    , we::type::Requirement const& capability_b
    , std::size_t num_worker_with_capability_b
    )
  {
    we::type::net_type net;

    {
      we::place_id_type const place_id
        (add_transition_with_requirement_and_input_place (net, capability_a));

      while (num_worker_with_capability_a --> 0)
      {
        net.put_value (place_id, we::type::literal::control());
      }
    }

    {
      we::place_id_type const place_id
        (add_transition_with_requirement_and_input_place (net, capability_b));

      while (num_worker_with_capability_b --> 0)
      {
        net.put_value (place_id, we::type::literal::control());
      }
    }

    return we::type::Activity
      ( we::type::Transition ( fhg::util::testing::random_string()
                               , net
                               , ::boost::none
                               , we::type::property::type()
                               , we::priority_type()
                               , ::boost::optional<we::type::eureka_id_type>{}
                               , std::list<we::type::Preference>{}
                               )
      );
  }

  [[noreturn]] void disallow (std::string what)
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
      , _layer ( std::bind
               (&wfe_and_counter_of_submitted_requirements::submit, this, std::placeholders::_2)
               , std::bind (&disallow, "cancel")
               , std::bind (&disallow, "finished")
               , std::bind (&disallow, "failed")
               , std::bind (&disallow, "canceled")
               , std::bind (&disallow, "token_put")
               , std::bind (&disallow, "workflow_response")
               , std::bind
                 (&wfe_and_counter_of_submitted_requirements::generate_id, this)
               , _random_extraction_engine
               )
    {}

    void submit (we::type::Activity activity)
    {
      const std::list<we::type::Requirement> list_req
        (activity.requirements_and_preferences (nullptr).requirements());

      BOOST_REQUIRE_EQUAL (list_req.size(), 1);

      std::lock_guard<std::mutex> const _ (_mtx_submitted);
      ++_received_requirements[list_req.front()];
      --_expected_activities;
      _cond_submitted.notify_one();
    }

    void wait_all_submitted()
    {
      std::unique_lock<std::mutex> lock (_mtx_submitted);
      _cond_submitted.wait (lock, [this]() { return !_expected_activities; });
    }

  private:
    std::mutex _mtx_submitted;
    std::condition_variable _cond_submitted;
    unsigned int _expected_activities;

  public:
    std::unordered_map<we::type::Requirement, unsigned int>
      _received_requirements;

  private:
    std::mt19937 _random_extraction_engine;

    std::mutex _generate_id_mutex;
    unsigned long _cnt {0};
    we::layer::id_type generate_id()
    {
      std::lock_guard<std::mutex> const _ (_generate_id_mutex);
      return std::to_string (++_cnt);
    }

  public:
    we::layer _layer;
  };
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_AUTO_TEST_CASE (layer_properly_forwards_requirements)
{
  wfe_and_counter_of_submitted_requirements helper (30);

  const we::type::Requirement req_a ("A");
  const we::type::Requirement req_b ("B");

  helper._layer.submit
    ( fhg::util::testing::random_string()
    , net_with_two_childs_that_require_capabilities (req_a, 20, req_b, 10)
    );
  helper.wait_all_submitted();

  BOOST_REQUIRE_EQUAL (helper._received_requirements.size(), 2);
  BOOST_REQUIRE_EQUAL (helper._received_requirements.at (req_a), 20);
  BOOST_REQUIRE_EQUAL (helper._received_requirements.at (req_b), 10);
}

namespace
{
  we::type::MultiModuleCall create_dummy_multi_mod
    (std::list<we::type::Preference> const& preferences)
  {
    we::type::MultiModuleCall multi_mod;

    for (auto const& target : preferences)
    {
      multi_mod.emplace
        ( target
        , we::type::ModuleCall
          ( fhg::util::testing::random_string()
          , fhg::util::testing::random_string()
          , std::unordered_map<std::string, we::type::MemoryBufferInfo>()
          , std::list<we::type::memory_transfer>()
          , std::list<we::type::memory_transfer>()
          , true
          , true
          )
        );
    }

    return multi_mod;
  }

  we::type::Activity activity_with_preferences
   (std::list<we::type::Preference> const& preferences)
  {
    we::type::Transition transition
      ( fhg::util::testing::random_string()
      , create_dummy_multi_mod (preferences)
      , ::boost::none
      , we::type::property::type()
      , we::priority_type()
      , ::boost::none
      , preferences
      );

   const std::string port_name (fhg::util::testing::random_string());
   transition.add_port ( we::type::Port ( port_name
                                          , we::type::port::direction::In{}
                                          , std::string ("string")
                                          , we::type::property::type()
                                          )
                       );

   we::type::Activity activity (transition);
   activity.add_input ( port_name
                      , fhg::util::testing::random_string_without ("\\\"")
                      );

   return activity;
  }

  class wfe_remembering_submitted_preferences
  {
  public:
    wfe_remembering_submitted_preferences()
      : _received_preferences()
      , _random_extraction_engine()
      , _layer ( std::bind
               (&wfe_remembering_submitted_preferences::submit, this, std::placeholders::_2)
               , std::bind (&disallow, "cancel")
               , std::bind (&disallow, "finished")
               , std::bind (&disallow, "failed")
               , std::bind (&disallow, "canceled")
               , std::bind (&disallow, "token_put")
               , std::bind (&disallow, "workflow_response")
               , std::bind
                   (&wfe_remembering_submitted_preferences::generate_id, this)
               , _random_extraction_engine
               )
    {}

    void submit (we::type::Activity const& activity)
    {
      _received_preferences = activity.preferences_TESTING_ONLY();
      std::lock_guard<std::mutex> const _ (_mtx_submitted);
      _submitted = true;
      _cond_submitted.notify_one();
    }

    void wait_submitted()
    {
      std::unique_lock<std::mutex> lock (_mtx_submitted);
      _cond_submitted.wait (lock, [&] { return _submitted; });
    }

    std::list<we::type::Preference> received_preferences()
    {
      return _received_preferences;
    }

  private:
    std::mutex _mtx_submitted;
    std::condition_variable _cond_submitted;
    bool _submitted {false};
    std::list<we::type::Preference> _received_preferences;

  private:
    std::mt19937 _random_extraction_engine;

    std::mutex _generate_id_mutex;
    unsigned long _cnt {0};
    we::layer::id_type generate_id()
    {
      std::lock_guard<std::mutex> const _ (_generate_id_mutex);
      return std::to_string (++_cnt);
    }

  public:
    we::layer _layer;
  };
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_AUTO_TEST_CASE (layer_properly_forwards_preferences)
{
  wfe_remembering_submitted_preferences wfe;

  auto const preferences
    ( fhg::util::testing::unique_randoms<std::list<we::type::Preference>>
        (fhg::util::testing::random<std::size_t>{} (10, 1))
    );

  wfe._layer.submit
    ( fhg::util::testing::random_string()
    , activity_with_preferences (preferences)
    );

  wfe.wait_submitted();

  BOOST_REQUIRE_EQUAL (wfe.received_preferences(), preferences);
}

namespace
{
  enum activity_type { child, eureka, no_eureka };

  we::type::eureka_id_type const eureka_id_1 ("group1");
  we::type::eureka_id_type const eureka_id_2 ("group2");
  we::type::eureka_id_type const eureka_id_3 ("group3");

  struct child_activity_with_eureka
  {
    we::type::Activity child;
    we::type::Activity result_eureka;
    we::type::Activity result_no_eureka;

    child_activity_with_eureka
      ( we::type::Transition const& t
      , we::transition_id_type const& t_id
      , std::set<we::type::eureka_id_type> const& h_set
      )
      : child (we::type::TESTING_ONLY{}, t, t_id)
      , result_eureka (child)
      , result_no_eureka (child)
    {
      child.add_input
        ( "in"
        , value::CONTROL
        );
      result_eureka.add_output_TESTING_ONLY
        ( "out"
        , pnet::type::value::wrap (h_set)
        );
    }
  };

  struct activity_with_transitions
  {
    we::type::net_type net;

    we::type::Activity input;
    we::type::Activity output;

    std::unordered_map < std::string
                       , child_activity_with_eureka
                       > activities;

    activity_with_transitions() = default;

    void add_transition_and_create_child_activity
      ( std::string place_name
      , we::type::eureka_id_type const eureka_id
      , std::set<we::type::eureka_id_type> const eurekaed_set
      )
    {
      std::ostringstream oss;
      oss << pnet::type::value::show (pnet::type::value::value_type (eureka_id));
      we::type::Transition transition
        ( "module_call"
        , we::type::ModuleCall
          ( "m"
          , "f"
          , std::unordered_map<std::string, we::type::MemoryBufferInfo>()
          , std::list<we::type::memory_transfer>()
          , std::list<we::type::memory_transfer>()
          , true
          , true
          )
        , ::boost::none
        , we::type::property::type()
        , we::priority_type()
        , oss.str()
        , std::list<we::type::Preference>{}
        );

      we::port_id_type const port_id_in
        ( transition.add_port ( we::type::Port ( "in"
                                                 , we::type::port::direction::In{}
                                                 , signature::CONTROL
                                                 , we::type::property::type()
                                                 )
                              )
        );
      we::port_id_type const port_id_out
        ( transition.add_port ( we::type::Port ( "out"
                                                 , we::type::port::direction::Out{}
                                                 , signature::SET
                                                 , we::type::property::type()
                                                 )
                              )
        );

      we::place_id_type place_id_in
        (net.add_place (place::type ( place_name
                                    , signature::CONTROL
                                    , true
                                    , we::type::property::type{}
                                    )
                       )
        );

      we::transition_id_type transition_id (net.add_transition (transition));
      {
        we::type::property::type empty;

        net.add_eureka (transition_id, port_id_out);
        net.add_connection ( we::edge::PT{}
                           , transition_id
                           , place_id_in
                           , port_id_in
                           , empty
                           );
      }

      activities.emplace ( place_name
                         , child_activity_with_eureka ( transition
                                                      , transition_id
                                                      , eurekaed_set
                                                      )
                         );
    }

    void create_job (std::size_t token_count_)
    {
      we::type::net_type copy_of_net_without_inputs (net);

      output = we::type::Activity
        ( we::type::Transition ( "net"
                                 , copy_of_net_without_inputs
                                 , ::boost::none
                                 , we::type::property::type()
                                 , we::priority_type()
                                 , ::boost::optional<we::type::eureka_id_type>{}
                                 , std::list<we::type::Preference>{}
                                 )
        );

      for (auto const& act : activities)
      {
        for (std::size_t i (0); i < token_count_; ++i)
        {
          net.put_token (act.first, value::CONTROL);
        }
      }

      input = we::type::Activity
        ( we::type::Transition ( "net"
                                 , net
                                 , ::boost::none
                                 , we::type::property::type()
                                 , we::priority_type()
                                 , ::boost::optional<we::type::eureka_id_type>{}
                                 , std::list<we::type::Preference>{}
                                 )
        );
    }

    we::type::Activity const& get_activity ( std::string name
                                             , activity_type type
                                             )
    {
      auto const& acts = activities.find (name);
      if (acts == activities.end())
      {
        throw std::logic_error ("missing child activitiy");
      }

      if (type == child)
      {
        return acts->second.child;
      }
      else if (type == eureka)
      {
        return acts->second.result_eureka;
      }
      else if (type == no_eureka)
      {
        return acts->second.result_no_eureka;
      }
      else
      {
        throw std::logic_error ("no child activitiy for given type");
      }
    }
  };
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (first_child_task_calls_eureka, daemon)
{
  activity_with_transitions test_job;
  test_job.add_transition_and_create_child_activity
    ( "A"
    , eureka_id_1
    , {eureka_id_1}
    );
  test_job.create_job (2);

  we::layer::id_type const id (generate_id());
  we::layer::id_type child_id_a;
  we::layer::id_type child_id_b;

  {
    expect_submit const _a
      (this, &child_id_a, test_job.get_activity ("A", child));
    expect_submit const _b
      (this, &child_id_b, test_job.get_activity ("A", child));

    do_submit (id, test_job.input);
  }

  //! \note assuming one child is still running
  {
    expect_cancel const _b_cancel (this, child_id_b);

    do_finished ( child_id_a
                , test_job.get_activity ("A", eureka)
                );
  }

  {
    expect_finished const _finish (this, id, test_job.output);

    do_canceled (child_id_b);
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (no_child_task_calls_eureka, daemon)
{
  activity_with_transitions test_job;
  test_job.add_transition_and_create_child_activity
    ( "A"
    , eureka_id_1
    , {eureka_id_1}
    );
  test_job.create_job (2);

  we::layer::id_type const id (generate_id());
  we::layer::id_type child_id_a;
  we::layer::id_type child_id_b;

  {
    expect_submit const _a
      (this, &child_id_a, test_job.get_activity ("A", child));
    expect_submit const _b
      (this, &child_id_b, test_job.get_activity ("A", child));

    do_submit (id, test_job.input);
  }

  do_finished (child_id_a, test_job.get_activity ("A", no_eureka));
  //! \note no tasks eureka, so normal exit
  {
    expect_finished const _finish (this, id, test_job.output);

    do_finished ( child_id_b
                , test_job.get_activity ("A", no_eureka)
                );
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (last_of_the_child_tasks_call_eureka, daemon)
{
  activity_with_transitions test_job;
  test_job.add_transition_and_create_child_activity
    ( "A"
    , eureka_id_1
    , {eureka_id_1}
    );
  test_job.create_job (2);

  we::layer::id_type const id (generate_id());
  we::layer::id_type child_id_a;
  we::layer::id_type child_id_b;

  {
    expect_submit const _a
      (this, &child_id_a, test_job.get_activity ("A", child));
    expect_submit const _b
      (this, &child_id_b, test_job.get_activity ("A", child));

    do_submit (id, test_job.input);
  }

  do_finished (child_id_a, test_job.get_activity ("A", no_eureka));
  //! \note no tasks eureka, so normal exit
  {
    expect_finished const _finish (this, id, test_job.output);

    do_finished ( child_id_b
                , test_job.get_activity ("A", eureka)
                );
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (one_of_two_groups_call_eureka, daemon)
{
  activity_with_transitions test_job;

  //! \note first activity with group 1
  test_job.add_transition_and_create_child_activity
    ( "A"
    , eureka_id_1
    , {eureka_id_1}
    );

  //! \note second activity with group 2
  test_job.add_transition_and_create_child_activity
    ( "B"
    , eureka_id_2
    , {eureka_id_2}
    );

  //! \note third activity with group 1
  test_job.add_transition_and_create_child_activity
    ( "C"
    , eureka_id_1
    , {eureka_id_1}
    );

  test_job.create_job (2);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id_A_1;
  we::layer::id_type child_id_A_2;
  we::layer::id_type child_id_B_1;
  we::layer::id_type child_id_B_2;
  we::layer::id_type child_id_C_1;
  we::layer::id_type child_id_C_2;

  {
    expect_submit const _a1
      (this, &child_id_A_1, test_job.get_activity ("A", child));
    expect_submit const _b1
      (this, &child_id_B_1, test_job.get_activity ("B", child));
    expect_submit const _c1
      (this, &child_id_C_1, test_job.get_activity ("C", child));
    expect_submit const _a2
      (this, &child_id_A_2, test_job.get_activity ("A", child));
    expect_submit const _b2
      (this, &child_id_B_2, test_job.get_activity ("B", child));
    expect_submit const _c2
      (this, &child_id_C_2, test_job.get_activity ("C", child));

    do_submit (id, test_job.input);
  }

  //! \note assuming one child of group 1 (A or C) eureka-ed
  {
    expect_cancel const _c2_cancel (this, child_id_C_2);
    expect_cancel const _a2_cancel (this, child_id_A_2);
    expect_cancel const _c1_cancel (this, child_id_C_1);

    do_finished (child_id_A_1, test_job.get_activity ("A", eureka));
  }

  //! \note no tasks eureka for B, so normal exit
  do_finished (child_id_B_1, test_job.get_activity ("B", no_eureka));
  {
    expect_finished const _ (this, id, test_job.output);

    do_canceled (child_id_C_2);
    do_canceled (child_id_A_2);
    do_canceled (child_id_C_1);
    do_finished ( child_id_B_2
                , test_job.get_activity ("B", no_eureka)
                );
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (two_of_three_groups_call_eureka, daemon)
{
  activity_with_transitions test_job;

  //! \note first activity with group 1, eurekas {1,3}
  test_job.add_transition_and_create_child_activity
    ( "A"
    , eureka_id_1
    , {eureka_id_1, eureka_id_3}
    );

  //! \note second activity with group 2, eurekas {2}
  test_job.add_transition_and_create_child_activity
    ( "B"
    , eureka_id_2
    , {eureka_id_2}
    );

  //! \note third activity with group 3, eurekas {3}
  test_job.add_transition_and_create_child_activity
    ( "C"
    , eureka_id_3
    , {eureka_id_3}
    );

  test_job.create_job (2);

  we::layer::id_type const id (generate_id());

  we::layer::id_type child_id_A_1;
  we::layer::id_type child_id_A_2;
  we::layer::id_type child_id_B_1;
  we::layer::id_type child_id_B_2;
  we::layer::id_type child_id_C_1;
  we::layer::id_type child_id_C_2;

  {
    expect_submit const _a1
      (this, &child_id_A_1, test_job.get_activity ("A", child));
    expect_submit const _b1
      (this, &child_id_B_1, test_job.get_activity ("B", child));
    expect_submit const _c1
      (this, &child_id_C_1, test_job.get_activity ("C", child));
    expect_submit const _a2
      (this, &child_id_A_2, test_job.get_activity ("A", child));
    expect_submit const _b2
      (this, &child_id_B_2, test_job.get_activity ("B", child));
    expect_submit const _c2
      (this, &child_id_C_2, test_job.get_activity ("C", child));

    do_submit (id, test_job.input);
  }

  //! \note assuming one child of group 1 (A or C) eureka-ed
  {
    expect_cancel const _c2_cancel (this, child_id_C_2);
    expect_cancel const _a2_cancel (this, child_id_A_2);
    expect_cancel const _c1_cancel (this, child_id_C_1);

    do_finished (child_id_A_1, test_job.get_activity ("A", eureka));
  }

  //! \note eureka tasks canceled, rest finish and exit
  do_finished (child_id_B_1, test_job.get_activity ("B", no_eureka));
  {
    expect_finished const _finish (this, id, test_job.output);

    do_canceled (child_id_C_2);
    do_canceled (child_id_A_2);
    do_canceled (child_id_C_1);
    do_finished ( child_id_B_2
                , test_job.get_activity ("B", no_eureka)
                );
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (one_of_two_eureka_jobs_call_eureka, daemon)
{
  activity_with_child (0);
  activity_with_transitions test_job_A;
  test_job_A.add_transition_and_create_child_activity
    ( "A"
    , eureka_id_1
    , {eureka_id_1}
    );
  test_job_A.create_job (2);

  activity_with_transitions test_job_B;
  test_job_B.add_transition_and_create_child_activity
    ( "B"
    , eureka_id_1
    , {eureka_id_1}
    );
  test_job_B.create_job (2);

  we::layer::id_type const id_A (generate_id());
  we::layer::id_type const id_B (generate_id());

  we::layer::id_type child_id_A_1;
  we::layer::id_type child_id_A_2;
  we::layer::id_type child_id_B_1;
  we::layer::id_type child_id_B_2;

  //! \note starting job A with two tokens to fire
  {
    expect_submit const _a1
      (this, &child_id_A_1, test_job_A.get_activity ("A", child));
    expect_submit const _a2
      (this, &child_id_A_2, test_job_A.get_activity ("A", child));

    do_submit (id_A, test_job_A.input);
  }

  //! \note starting separate job B with same eureka ID as job A
  {
    expect_submit const _b1
      (this, &child_id_B_1, test_job_B.get_activity ("B", child));
    expect_submit const _b2
      (this, &child_id_B_2, test_job_B.get_activity ("B", child));

    do_submit (id_B, test_job_B.input);
  }

  //! \note assuming one child of job A eureka-ed
  {
    expect_cancel const _a2_cancel (this, child_id_A_2);

    do_finished ( child_id_A_1
                , test_job_A.get_activity ("A", eureka)
                );
  }

  //! \note finish job A with a eureka
  {
    expect_finished const _a_finish (this, id_A, test_job_A.output);

    do_canceled (child_id_A_2);
  }

  //! \note finish job B normally
  do_finished ( child_id_B_1
              , test_job_B.get_activity ("B", no_eureka)
              );
  {
    expect_finished const _b_finish (this, id_B, test_job_B.output);

    do_finished ( child_id_B_2
                , test_job_B.get_activity ("B", no_eureka)
                );
  }
}

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_FIXTURE_TEST_CASE
  (calling_eureka_on_failed_child_tasks, daemon)
{
  activity_with_transitions test_job;
  test_job.add_transition_and_create_child_activity
    ( "A"
    , eureka_id_1
    , {eureka_id_1}
    );
  test_job.create_job (2);

  we::layer::id_type const id (generate_id());
  we::layer::id_type child_id_a;
  we::layer::id_type child_id_b;

  {
    expect_submit const _a
      (this, &child_id_a, test_job.get_activity ("A", child));
    expect_submit const _b
      (this, &child_id_b, test_job.get_activity ("A", child));

    do_submit (id, test_job.input);
  }

  //! \note assuming one child is still running, and fails
  {
    expect_cancel const _b_cancel (this, child_id_b);

    do_finished ( child_id_a
                , test_job.get_activity ("A", eureka)
                );
  }

  std::string const fail_reason (fhg::util::testing::random_string());
  {
    expect_finished const _finish (this, id, test_job.output);

    do_failed (child_id_b, fail_reason);
  }
}
