#define BOOST_TEST_MODULE FhgUtilAtomicCounter
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <vector>
#include <boost/thread.hpp>

#include <fhg/util/thread/atomic.hpp>

BOOST_AUTO_TEST_CASE (single_thread_increment)
{
  using namespace fhg::thread;
  atomic<int> atom;

  BOOST_REQUIRE_EQUAL (atom, 0);

  ++atom;
  BOOST_REQUIRE_EQUAL (atom, 1);

  atom++;
  BOOST_REQUIRE_EQUAL (atom, 2);

  int v = atom++;
  BOOST_REQUIRE_EQUAL (v, 2);
  BOOST_REQUIRE_EQUAL (atom, 3);
}

static void s_increment (fhg::thread::atomic<int> & a, int N)
{
  while (N-->0)
    ++a;
}

BOOST_AUTO_TEST_CASE (threaded_increment)
{
  static const size_t NUM_THREADS = 4;
  static const size_t NUM_ITERATIONS = 1000000;

  using namespace fhg::thread;
  atomic<int> atom;

  BOOST_REQUIRE_EQUAL (atom, 0);

  std::vector<boost::shared_ptr<boost::thread> > v;
  for (size_t i = 0 ; i < NUM_THREADS ; ++i)
    v.push_back(boost::shared_ptr<boost::thread>
               (new boost::thread ( s_increment
                                  , boost::ref (atom)
                                  , NUM_ITERATIONS
                                  )
               ));

  for (size_t i = 0 ; i < NUM_THREADS ; ++i)
    v[i]->join ();

  BOOST_REQUIRE_EQUAL (atom, NUM_THREADS * NUM_ITERATIONS);
}

static void s_decrement (fhg::thread::atomic<int> & a, int N)
{
  while (N-->0)
    --a;
}

BOOST_AUTO_TEST_CASE (threaded_decrement)
{
  static const size_t NUM_THREADS = 4;
  static const size_t NUM_ITERATIONS = 1000000;

  using namespace fhg::thread;
  atomic<int> atom (NUM_THREADS * NUM_ITERATIONS);

  std::vector<boost::shared_ptr<boost::thread> > v;
  for (size_t i = 0 ; i < NUM_THREADS ; ++i)
    v.push_back(boost::shared_ptr<boost::thread>
               (new boost::thread ( s_decrement
                                  , boost::ref (atom)
                                  , NUM_ITERATIONS
                                  )
               ));

  for (size_t i = 0 ; i < NUM_THREADS ; ++i)
    v[i]->join ();

  BOOST_REQUIRE_EQUAL (atom, 0);
}

BOOST_AUTO_TEST_CASE (threaded_inc_dec_mixed)
{
  static const size_t NUM_INC_THREADS = 3;
  static const size_t NUM_DEC_THREADS = 1;
  static const size_t NUM_ITERATIONS =  1000000;

  using namespace fhg::thread;
  atomic<int> atom (0);

  std::vector<boost::shared_ptr<boost::thread> > v;
  for (size_t i = 0 ; i < NUM_INC_THREADS ; ++i)
    v.push_back(boost::shared_ptr<boost::thread>
               (new boost::thread ( s_increment
                                  , boost::ref (atom)
                                  , NUM_ITERATIONS
                                  )
               ));
  for (size_t i = 0 ; i < NUM_DEC_THREADS ; ++i)
    v.push_back(boost::shared_ptr<boost::thread>
               (new boost::thread ( s_decrement
                                  , boost::ref (atom)
                                  , NUM_ITERATIONS
                                  )
               ));

  for (size_t i = 0 ; i < v.size () ; ++i)
    v[i]->join ();

  BOOST_REQUIRE_EQUAL ( atom
                      , NUM_INC_THREADS * NUM_ITERATIONS
                      - NUM_DEC_THREADS * NUM_ITERATIONS
                      );
}
