#define BOOST_TEST_MODULE GspcKvsImplTests
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <errno.h>
#include <stdlib.h>

#include <algorithm>    // std::sort
#include <iostream>

#include <fhg/util/now.hpp>
#include <boost/format.hpp>

#include <we/type/value/peek.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <gspc/kvs/api.hpp>
#include <gspc/kvs/kvs.hpp>
#include <gspc/kvs/impl/kvs_impl.hpp>

BOOST_AUTO_TEST_CASE (test_impl_invalid_key)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  rc = kvs.get ("foo bar", val);
  BOOST_REQUIRE_EQUAL (rc, -EKEYREJECTED);
}

BOOST_AUTO_TEST_CASE (test_impl_put_get_del)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  rc = kvs.get ("foo", val);

  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);

  rc = kvs.put ("foo", 42);

  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.get ("foo", val);

  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 42);

  rc = kvs.put ("foo", std::string ("bar"));
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.get ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);

  BOOST_REQUIRE_EQUAL ( std::string (boost::get<std::string>(val))
                      , "bar"
                      );

  rc = kvs.del ("foo");

  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.del ("foo");

  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);

  rc = kvs.get ("foo", val);

  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);
}

BOOST_AUTO_TEST_CASE (test_impl_push_try_pop)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, -EAGAIN);

  rc = kvs.put ("foo", 42);
  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, -EINVAL);

  rc = kvs.del ("foo");
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.push ("foo", 1);
  BOOST_REQUIRE_EQUAL (rc, 0);
  rc = kvs.push ("foo", 2);
  BOOST_REQUIRE_EQUAL (rc, 0);
  rc = kvs.push ("foo", 3);
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 1);

  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 2);

  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 3);
}

static void s_push_value ( gspc::kvs::api_t *kvs
                         , gspc::kvs::api_t::key_type const &key
                         , gspc::kvs::api_t::value_type const &val
                         )
{
  usleep (rand () % 1500);
  kvs->push (key, val);
}

BOOST_AUTO_TEST_CASE (test_impl_push_pop)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  rc = kvs.push ("foo", 1);
  BOOST_REQUIRE_EQUAL (rc, 0);
  rc = kvs.push ("foo", 2);
  BOOST_REQUIRE_EQUAL (rc, 0);
  rc = kvs.push ("foo", 3);
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 1);

  rc = kvs.pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 2);

  rc = kvs.pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 3);

  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, -EAGAIN);

  rc = kvs.pop ("foo", val, 500);
  BOOST_REQUIRE_EQUAL (rc, -ETIME);

  // fire up a thread to push
  gspc::kvs::api_t::value_type val_to_push (std::string ("bar"));
  boost::thread pusher (boost::bind (&s_push_value, &kvs, "foo", val_to_push));

  rc = kvs.pop ("foo", val, 1500);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<std::string>(val), "bar");

  pusher.join ();
}

BOOST_AUTO_TEST_CASE (test_impl_wait)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  rc = kvs.wait ("foo", gspc::kvs::api_t::E_EXIST, 500);
  BOOST_REQUIRE_EQUAL (rc, -ETIME);

  // fire up a thread to push
  gspc::kvs::api_t::value_type val_to_push (std::string ("bar"));
  boost::thread pusher (boost::bind (&s_push_value, &kvs, "foo", val_to_push));

  rc = kvs.wait ("foo", gspc::kvs::api_t::E_PUSH, 2000);
  BOOST_REQUIRE (rc & gspc::kvs::api_t::E_PUSH);

  pusher.join ();

  rc = kvs.pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<std::string>(val), "bar");
}

BOOST_AUTO_TEST_CASE (test_impl_reset_inc_dec)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  int val;

  rc = kvs.counter_increment ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (val, 1);

  rc = kvs.counter_decrement ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (val, 0);

  rc = kvs.counter_reset ("foo", 0);
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.counter_increment ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (val, 1);

  rc = kvs.counter_change ("foo", val, 2);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (val, 3);

  rc = kvs.counter_decrement ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (val, 2);

  rc = kvs.counter_change ("foo", val, -2);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (val, 0);

  rc = kvs.counter_change ("foo", val, 0);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (val, 0);

  rc = kvs.del ("foo");
}

struct compare_first
{
  template <typename T>
  bool operator()(T const &a, T const &b)
  {
    return a.first < b.first;
  }
};

BOOST_AUTO_TEST_CASE (test_impl_get_regex)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  std::list<std::pair< gspc::kvs::kvs_t::key_type
                     , gspc::kvs::kvs_t::value_type
                     > > values;

  kvs.put ("foo.1", "bar");
  kvs.put ("foo.2", "bar");
  kvs.put ("foo.3", "bar");
  kvs.put ("foo.bar", "bar");

  rc = kvs.get_regex ("^foo\\.[0-9]$", values);
  BOOST_REQUIRE_EQUAL (rc, 0);

  BOOST_REQUIRE_EQUAL (values.size (), 3u);

  std::list<std::pair< gspc::kvs::kvs_t::key_type
                     , gspc::kvs::kvs_t::value_type
                     > >::const_iterator it = values.begin ();
  std::list<std::pair< gspc::kvs::kvs_t::key_type
                     , gspc::kvs::kvs_t::value_type
                     > >::const_iterator end = values.end ();

  while (it != end)
  {
    BOOST_REQUIRE_NE (it->first, "foo.bar");
    ++it;
  }
}

BOOST_AUTO_TEST_CASE (test_impl_del_regex)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::kvs_t::value_type val;

  kvs.put ("foo.1", "bar");
  kvs.put ("foo.2", "bar");
  kvs.put ("foo.3", "bar");
  kvs.put ("foo.bar", std::string ("bar"));

  rc = kvs.del_regex ("^foo\\.[0-9]$");
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.get ("foo.1", val);
  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);
  rc = kvs.get ("foo.2", val);
  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);
  rc = kvs.get ("foo.3", val);
  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);

  rc = kvs.get ("foo.bar", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<std::string>(val), "bar");
}

BOOST_AUTO_TEST_CASE (test_impl_expiry)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  kvs.put ("foo.1", "bar");
  kvs.set_ttl ("foo.1", 1);

  sleep (1);

  rc = kvs.get ("foo.1", val);
  BOOST_REQUIRE_EQUAL (rc, -EKEYEXPIRED);
}

static void s_wfh_client_thread ( const size_t rank
                                , gspc::kvs::api_t *kvs
                                , const size_t nmsg
                                , const std::string &queue
                                )
{
  const std::string my_queue ((boost::format ("thread-%1%") % rank).str ());

  for (size_t i = 0 ; i < nmsg ; ++i)
  {
    pnet::type::value::value_type rqst
      = pnet::type::value::read
      ((boost::format ( "Struct [from := \"%1%\", msg := %2%]"
                      ) % my_queue % i
       ).str ());
    pnet::type::value::value_type rply;

    BOOST_REQUIRE_EQUAL (kvs->push (queue, rqst), 0);
    BOOST_REQUIRE_EQUAL (kvs->pop (my_queue, rply, 10 * 1000), 0);
  }
}

BOOST_AUTO_TEST_CASE (test_impl_many_push_pop)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;
  const std::string queue ("wfh");

  static const size_t NUM = 10;
  static const size_t NTHREAD = 15;

  std::vector<boost::shared_ptr<boost::thread> >
    threads;

  for (size_t i = 0 ; i < NTHREAD ; ++i)
  {
    threads.push_back
      (boost::shared_ptr<boost::thread>
      (new boost::thread (boost::bind ( &s_wfh_client_thread
                                      , i
                                      , &kvs
                                      , NUM
                                      , queue
                                      )
                         )
      ));
  }

  for (size_t i = 0 ; i < NTHREAD*NUM ; ++i)
  {
    rc = kvs.pop (queue, val, 1000);
    if (rc != 0)
    {
      std::cerr << "wfh: could not pop #" << i << " from '" << queue << "': "
                << strerror (-rc)
                << std::endl
        ;

      rc = kvs.get (queue, val);
      if (rc == 0)
      {
        std::cerr << "wfh: queue content: "
                  << pnet::type::value::show (val)
                  << std::endl
          ;
      }
      else
      {
        std::cerr << "wfh: could not get queue content: "
                  << strerror (-rc)
                  << std::endl
          ;
      }

      break;
    }

    std::string from =
      boost::get<std::string>(*pnet::type::value::peek ("from", val));
    int msg =
      boost::get<int>(*pnet::type::value::peek ("msg", val));

    rc = kvs.push (from, msg);
    if (rc != 0)
    {
      std::cerr << "wfh: could not push #" << msg << " to '" << from << "': "
                << strerror (-rc)
                << std::endl
        ;
      break;
    }

    std::cerr << "wfh: sent reply " << i+1 << "/" << NTHREAD*NUM
              << " to '" << from << "'"
              << std::endl
      ;
  }

  if (0 == rc)
  {
    std::cerr << "wfh: everything done" << std::endl;
  }
  else
  {
    std::cerr << "wfh: failed: " << strerror (-rc) << std::endl;
  }

  for (size_t i = 0 ; i < NTHREAD ; ++i)
  {
    threads [i]->join ();
  }

  BOOST_REQUIRE_EQUAL (rc, 0);
}

BOOST_AUTO_TEST_CASE (test_global_kvs)
{
  static const size_t NUM = 50;

  int rc;
  gspc::kvs::api_t::value_type val;

  for (size_t i = 0 ; i < NUM ; ++i)
  {
    rc = gspc::kvs::initialize ("inproc://");
    BOOST_REQUIRE_EQUAL (rc, 0);

    rc = gspc::kvs::get ().get ("foo", val);
    BOOST_REQUIRE_EQUAL (rc, -ENOKEY);

    rc = gspc::kvs::get ().put ("foo", "bar");
    BOOST_REQUIRE_EQUAL (rc, 0);

    rc = gspc::kvs::get ().get ("foo", val);
    BOOST_REQUIRE_EQUAL (rc, 0);
    BOOST_REQUIRE_EQUAL ("bar", boost::get<std::string>(val));

    rc = gspc::kvs::get ().del ("foo");
    BOOST_REQUIRE_EQUAL (rc, 0);

    rc = gspc::kvs::shutdown ();
    BOOST_REQUIRE_EQUAL (rc, 0);
  }
}
