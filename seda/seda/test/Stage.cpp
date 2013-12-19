#define BOOST_TEST_MODULE SedaStage
#include <boost/test/unit_test.hpp>

#include <seda/Stage.hpp>

namespace
{
  struct wait_for_n_events_strategy
  {
    wait_for_n_events_strategy (unsigned int expected)
      : _counter (0)
      , _expected (expected)
    {}

    void perform (const boost::shared_ptr<int>&)
    {
      boost::mutex::scoped_lock _ (_counter_mutex);
      ++_counter;

      BOOST_REQUIRE_LE (_counter, _expected);
      if (_counter == _expected)
      {
        _expected_count_reached.notify_all();
      }
    }
    void wait() const
    {
      boost::mutex::scoped_lock _ (_counter_mutex);

      while (_counter < _expected)
      {
        _expected_count_reached.wait (_);
      }

      BOOST_REQUIRE_EQUAL (_counter, _expected);
    }

    mutable boost::mutex _counter_mutex;
    mutable boost::condition_variable _expected_count_reached;
    unsigned int _counter;
    unsigned int _expected;
  };
}

BOOST_AUTO_TEST_CASE (send_n_messages)
{
  const std::size_t numMsgs(1000);

  wait_for_n_events_strategy counter (numMsgs);

  seda::Stage<int> stage
    (boost::bind (&wait_for_n_events_strategy::perform, &counter, _1));

  for (std::size_t i=0; i < numMsgs; ++i) {
    stage.send(boost::shared_ptr<int>(new int (i)));
  }

  counter.wait();
}
