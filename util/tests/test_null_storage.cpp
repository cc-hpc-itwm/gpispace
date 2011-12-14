#include <boost/test/included/unit_test.hpp>

#include <map>

#include <boost/serialization/map.hpp>

#include <stdlib.h>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/filesystem.hpp>


#include <fhg/plugin/core/null_storage.hpp>

using namespace boost::unit_test;

namespace fs = boost::filesystem;

void test_invalid_key ()
{
  using namespace fhg::plugin;

  BOOST_REQUIRE (! Storage::validate ("."));
  BOOST_REQUIRE (! Storage::validate ("/"));
  BOOST_REQUIRE (! Storage::validate ("/home"));
  BOOST_REQUIRE (! Storage::validate ("../../foo"));
  BOOST_REQUIRE (! Storage::validate ("foo.bar"));
  BOOST_REQUIRE (! Storage::validate ("foo/../"));
  BOOST_REQUIRE (! Storage::validate (".."));
}

void test_save_load_delete ()
{
  using namespace fhg::plugin::core;

  NullStorage s;
  s.set_auto_commit (false);

  {
    std::string text;

    BOOST_REQUIRE_EQUAL (s.save("foo", "bar"), 0);
    BOOST_REQUIRE_EQUAL (s.load("foo", text), 0);
    BOOST_REQUIRE_EQUAL (text, "bar");

    BOOST_REQUIRE_EQUAL (s.commit(), 0);
    BOOST_REQUIRE_EQUAL (s.flush(), 0);

    BOOST_REQUIRE_EQUAL (s.remove ("foo"), 0);
    BOOST_REQUIRE_EQUAL (s.load ("foo", text), -ENOENT);
  }

  {
    int i;

    BOOST_REQUIRE_EQUAL (s.save("bar", 42), 0);
    BOOST_REQUIRE_EQUAL (s.load("bar", i), 0);
    BOOST_REQUIRE_EQUAL (i, 42);

    BOOST_REQUIRE_EQUAL (s.commit(), 0);
    BOOST_REQUIRE_EQUAL (s.flush(), 0);

    BOOST_REQUIRE_EQUAL (s.remove ("bar"), 0);
    BOOST_REQUIRE_EQUAL (s.load ("bar", i), -ENOENT);
  }
}

void test_save_load_invalid ()
{
  using namespace fhg::plugin::core;

  NullStorage s;

  {
    std::string text;
    BOOST_REQUIRE_EQUAL(s.save("text", text), 0);
  }

  {
    int i;
    BOOST_REQUIRE_EQUAL(s.load("text", i), -EINVAL);
  }

  BOOST_REQUIRE_EQUAL(s.remove("text"), 0);
}

void test_hierachy ()
{
  using namespace fhg::plugin::core;
  NullStorage s;

  BOOST_REQUIRE (s.remove ("dummy") != -EISDIR);
  BOOST_REQUIRE_EQUAL (s.save("dummy", "dummy"), 0);

  BOOST_REQUIRE_EQUAL (s.add_storage ("dummy"), -ENOTDIR);

  BOOST_REQUIRE_EQUAL (s.add_storage ("sub1"), 0);
  {
    fhg::plugin::Storage *sub = s.get_storage ("sub1");
    BOOST_REQUIRE (sub);
    sub->save("dummy", "dummy");
    std::string dummy;
    sub->load("dummy", dummy);
    sub->remove("dummy");
    BOOST_REQUIRE_EQUAL (dummy, "dummy");
  }
  BOOST_REQUIRE_EQUAL (s.del_storage("sub1"), 0);
}

test_suite*
init_unit_test_suite (int ac, char *av[])
{
  test_suite* ts1 = BOOST_TEST_SUITE( "null_storage" );
  ts1->add( BOOST_TEST_CASE(&test_save_load_delete));
  ts1->add( BOOST_TEST_CASE(&test_hierachy));
  ts1->add( BOOST_TEST_CASE(&test_save_load_invalid));
  ts1->add( BOOST_TEST_CASE(&test_invalid_key));

  framework::master_test_suite().add( ts1 );

  return 0;
}
