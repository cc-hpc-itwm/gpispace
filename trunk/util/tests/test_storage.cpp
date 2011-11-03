#include <boost/test/included/unit_test.hpp>

#include <stdlib.h>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <fhg/plugin/core/file_storage.hpp>

using namespace boost::unit_test;

static std::string working_dir;

void test_simple_storage ()
{
  using namespace fhg::plugin::core;

  FileStorage s (working_dir);
  s.set_auto_commit (false);

  {
    std::string text;

    BOOST_REQUIRE_EQUAL (s.save("foo", "bar"), 0);
    BOOST_REQUIRE_EQUAL (s.load("foo", text), 0);
    BOOST_REQUIRE_EQUAL (text, "bar");
  }

  {
    int i;

    BOOST_REQUIRE_EQUAL (s.save("bar", 42), 0);
    BOOST_REQUIRE_EQUAL (s.load("bar", i), 0);
    BOOST_REQUIRE_EQUAL (i, 42);
  }
  BOOST_REQUIRE_EQUAL (s.commit(), 0);

  BOOST_REQUIRE_EQUAL (s.flush(), 0);

  BOOST_REQUIRE_EQUAL (s.remove ("foo"), 0);
  BOOST_REQUIRE_EQUAL (s.remove ("bar"), 0);

  BOOST_REQUIRE_EQUAL (s.commit(), 0);
}

void test_restore_storage ()
{
  using namespace fhg::plugin::core;

  {
    FileStorage s (working_dir);
    BOOST_REQUIRE_EQUAL (s.save ("question", "The answer to life, the universe and everything"), 0);
    BOOST_REQUIRE_EQUAL (s.save ("answer", 42), 0);
  }

  {
    FileStorage s (working_dir);
    BOOST_REQUIRE_EQUAL (s.restore(), 0);

    std::string q;
    BOOST_REQUIRE_EQUAL (s.load ("question", q), 0);

    int a;
    BOOST_REQUIRE_EQUAL (s.load ("answer", a), 0);
  }
}

void test_hierachical_storage ()
{
  using namespace fhg::plugin::core;
  FileStorage s (working_dir);
  BOOST_REQUIRE_EQUAL (s.add_storage ("sub1"), 0);
}

test_suite*
init_unit_test_suite (int ac, char *av[])
{
  if (ac <= 1)
  {
    throw boost::unit_test::framework::setup_error
      ("I need the path to a temporary dictory to work in!");
  }

  working_dir = av[1];

  {
    test_suite* ts1 = BOOST_TEST_SUITE( "basic_storage" );
    ts1->add( BOOST_TEST_CASE(&test_simple_storage));
    ts1->add( BOOST_TEST_CASE(&test_restore_storage));
    ts1->add( BOOST_TEST_CASE(&test_hierachical_storage));

    framework::master_test_suite().add( ts1 );
  }

  return 0;
}
