#define BOOST_TEST_MODULE UtilThreadChannelTest
#include <boost/test/unit_test.hpp>

#include <stdlib.h>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <fhg/util/thread/channel.hpp>
#include <fhg/util/thread/poll.hpp>

typedef fhg::thread::channel<int> int_channel_t;

BOOST_AUTO_TEST_CASE (thread_channel_in_out)
{
  int_channel_t chan (1);

  const int in = 1;
  short    out = 0;

  chan << in;
  chan >> out;

  BOOST_REQUIRE_EQUAL (in, out);
}

BOOST_AUTO_TEST_CASE (thread_channel_chain_in_out)
{
  int_channel_t chan;

  const int i[] = { 0,  1,  2,  3};
  int       o[] = {-1, -1, -1, -1};

  chan << i[0] << i[1] << i[2] << i[3];
  chan >> o[0] >> o[1] >> o[2] >> o[3];

  for (size_t idx = 0 ; idx < sizeof (i) / sizeof (int); ++idx)
  {
    BOOST_CHECK_EQUAL (i[idx], o[idx]);
  }
}

typedef std::pair<int, int> to_add_t;
typedef fhg::thread::channel<to_add_t> to_add_channel_t;

static void s_adder (to_add_channel_t & in, int_channel_t & out)
{
  for (;;)
  {
    to_add_t to_add;
    in >> to_add;
    out << (to_add.first + to_add.second);
  }
}

BOOST_AUTO_TEST_CASE (thread_channel_multiple_adder)
{
  static const int NUM_THREADS = 4;
  static const int VALUES_TO_ADD = 1 << 18;

  srand (1337);

  to_add_channel_t input;
  int_channel_t output;
  std::vector<int> reference;

  // fire up threads
  std::vector<boost::thread*> threads;
  for (int i = 0 ; i < NUM_THREADS ; ++i)
  {
    threads.push_back (new boost::thread ( s_adder
                                         , boost::ref (input)
                                         , boost::ref (output)
                                         )
                      );
  }

  // generate values and feed to workers
  for (int i = 0 ; i < VALUES_TO_ADD ; ++i)
  {
    to_add_t to_add = std::make_pair (rand (), rand ());
    input << to_add;
    reference.push_back (to_add.first + to_add.second);
  }

  // kinda busy wait ;-(
  while (output.size () < VALUES_TO_ADD)
  {
    usleep (500);
  }

  // join threads
  for (size_t i = 0 ; i < threads.size () ; ++i)
  {
    threads [i]->interrupt ();
    threads [i]->join ();
    delete threads [i];
  }
  threads.clear ();

  // compare result
  std::sort (reference.begin (), reference.end ());

  std::vector<int>::iterator it;
  while (not output.empty ())
  {
    int v = output.get ();
    it = std::lower_bound (reference.begin (), reference.end (), v);
    BOOST_REQUIRE (it != reference.end ());
    BOOST_CHECK_EQUAL (*it, v);
    reference.erase (it);
  }

  BOOST_REQUIRE (reference.empty ());
}

BOOST_AUTO_TEST_CASE (thread_channel_select_in_empty_channel)
{
  int_channel_t chan_a (1);
  int_channel_t chan_b (1);

  fhg::thread::poll_item_t items [] =
    {
      {&chan_a, 0, fhg::thread::POLLIN, 0}
    , {&chan_b, 0, fhg::thread::POLLIN, 0}
    };

  int nready = fhg::thread::poll (items, 2, 0);

  BOOST_REQUIRE (nready == 0);
  BOOST_CHECK (items [0].revents == 0);
  BOOST_CHECK (items [1].revents == 0);
}

BOOST_AUTO_TEST_CASE (thread_channel_select_in_full_channel)
{
  int_channel_t chan_a (1);
  int_channel_t chan_b (1);

  chan_a << 1;
  chan_b << 1;

  fhg::thread::poll_item_t items [] =
    {
      {&chan_a, 0, fhg::thread::POLLIN, 0}
    , {&chan_b, 0, fhg::thread::POLLIN, 0}
    };

  int nready = fhg::thread::poll (items, 2, 0);

  BOOST_REQUIRE (nready == 2);
  BOOST_CHECK (items [0].revents == fhg::thread::POLLIN);
  BOOST_CHECK (items [1].revents == fhg::thread::POLLIN);
}

BOOST_AUTO_TEST_CASE (thread_channel_select_out_empty_channel)
{
  int_channel_t chan_a (1);
  int_channel_t chan_b (1);

  fhg::thread::poll_item_t items [] =
    {
      {&chan_a, 0, fhg::thread::POLLOUT, 0}
    , {&chan_b, 0, fhg::thread::POLLOUT, 0}
    };

  int nready = fhg::thread::poll (items, 2, 0);

  BOOST_REQUIRE (nready == 2);
  BOOST_CHECK (items [0].revents == fhg::thread::POLLOUT);
  BOOST_CHECK (items [1].revents == fhg::thread::POLLOUT);
}

BOOST_AUTO_TEST_CASE (thread_channel_select_out_full_channel)
{
  int_channel_t chan_a (1);
  int_channel_t chan_b (1);

  chan_a << 1;
  chan_b << 1;

  fhg::thread::poll_item_t items [] =
    {
      {&chan_a, 0, fhg::thread::POLLOUT, 0}
    , {&chan_b, 0, fhg::thread::POLLOUT, 0}
    };

  int nready = fhg::thread::poll (items, 2, 0);

  BOOST_REQUIRE (nready == 0);
  BOOST_CHECK (items [0].revents == 0);
  BOOST_CHECK (items [1].revents == 0);
}

BOOST_AUTO_TEST_CASE (thread_channel_select_mixed_channel)
{
  int_channel_t chan_a (1);
  int_channel_t chan_b (1);

  chan_a << 1;

  fhg::thread::poll_item_t items [] =
    {
      {&chan_a, 0, fhg::thread::POLLIN + fhg::thread::POLLOUT, 0}
    , {&chan_b, 0, fhg::thread::POLLOUT + fhg::thread::POLLIN, 0}
    };

  int nready = fhg::thread::poll (items, 2, 0);

  BOOST_REQUIRE (nready == 2);
  BOOST_CHECK (items [0].revents == fhg::thread::POLLIN);
  BOOST_CHECK (items [1].revents == fhg::thread::POLLOUT);
}

static void s_forward_element ( int_channel_t *chan_a
                              , int_channel_t *chan_b
                              )
{
  usleep (100);
  *chan_a >> *chan_b >> *chan_a >> *chan_b;
}

BOOST_AUTO_TEST_CASE (thread_channel_select_timeout)
{
  int nready;
  int_channel_t chan_a (1);
  int_channel_t chan_b (1);

  fhg::thread::poll_item_t items [] =
    {
      {&chan_a, 0, fhg::thread::POLLIN, 0}
    , {&chan_b, 0, fhg::thread::POLLIN, 0}
    };

  nready = fhg::thread::poll (items, 2, 500 * FHG_THREAD_SELECT_MS);

  BOOST_REQUIRE (nready == 0);
}

BOOST_AUTO_TEST_CASE (thread_channel_select_mixed_timeout)
{
  int nready;
  int_channel_t chan_a (1);
  int_channel_t chan_b (1);

  chan_a << 42;

  fhg::thread::poll_item_t items [] =
    {
      {&chan_a, 0, fhg::thread::POLLOUT, 0}
    , {&chan_b, 0, fhg::thread::POLLIN, 0}
    };

  nready = fhg::thread::poll (items, 2, 500 * FHG_THREAD_SELECT_MS);
  BOOST_REQUIRE (nready == 0);

  boost::thread T = boost::thread (&s_forward_element, &chan_a, &chan_b);
  nready = fhg::thread::poll (items, 2, 5000 * FHG_THREAD_SELECT_MS);

  BOOST_REQUIRE (nready > 0);

  if (items [0].revents)
    BOOST_CHECK (items [0].revents == fhg::thread::POLLOUT);
  if (items [1].revents)
    BOOST_CHECK (items [1].revents == fhg::thread::POLLIN);

  int elem;
  chan_b >> elem;

  BOOST_REQUIRE_EQUAL (elem, 42);
}

BOOST_AUTO_TEST_CASE (thread_channel_drop)
{
  int nready;
  int_channel_t chan;

  for (int i = 0 ; i < 10 ; ++i)
    chan << i;

  chan >> fhg::thread::drop (5);

  for (int i = 5 ; i < 10 ; ++i)
  {
    int elem;
    chan >> elem;
    BOOST_CHECK_EQUAL (elem, i);
  }

  BOOST_REQUIRE (chan.empty ());
}
