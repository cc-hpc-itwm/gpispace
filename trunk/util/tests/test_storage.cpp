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


#include <fhg/plugin/core/file_storage.hpp>

using namespace boost::unit_test;

namespace fs = boost::filesystem;

static fs::path working_dir;

void test_invalid_key ()
{
  using namespace fhg::plugin::core;

  BOOST_REQUIRE (! FileStorage::validate ("."));
  BOOST_REQUIRE (! FileStorage::validate ("/"));
  BOOST_REQUIRE (! FileStorage::validate ("/home"));
  BOOST_REQUIRE (! FileStorage::validate ("../../foo"));
  BOOST_REQUIRE (! FileStorage::validate ("foo.bar"));
  BOOST_REQUIRE (! FileStorage::validate ("foo/../"));
  BOOST_REQUIRE (! FileStorage::validate (".."));
}

void test_save_load_delete ()
{
  using namespace fhg::plugin::core;

  FileStorage s (working_dir);
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

  FileStorage s (working_dir);

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

void test_restore ()
{
  using namespace fhg::plugin::core;

  {
    FileStorage s (working_dir);
    BOOST_REQUIRE_EQUAL (s.save ("question", "The answer to life, the universe and everything"), 0);
    BOOST_REQUIRE_EQUAL (s.commit(), 0);
    BOOST_REQUIRE_EQUAL (s.flush(), 0);
  }

  {
    FileStorage s (working_dir);
    BOOST_REQUIRE_EQUAL (s.restore(), 0);

    std::string q;
    BOOST_REQUIRE_EQUAL (s.load ("question", q), 0);
    BOOST_REQUIRE_EQUAL (q, "The answer to life, the universe and everything");
    BOOST_REQUIRE_EQUAL (s.save ("answer", 42), 0);
    BOOST_REQUIRE_EQUAL (s.commit(), 0);
    BOOST_REQUIRE_EQUAL (s.flush(), 0);
  }

  {
    FileStorage s (working_dir);
    BOOST_REQUIRE_EQUAL (s.restore(), 0);

    int a;
    BOOST_REQUIRE_EQUAL (s.load ("answer", a), 0);
    BOOST_REQUIRE_EQUAL (a, 42);
  }
}

void test_hierachy ()
{
  using namespace fhg::plugin::core;
  FileStorage s (working_dir);

  BOOST_REQUIRE (s.remove ("dummy") != -EISDIR);
  BOOST_REQUIRE_EQUAL (s.save("dummy", "dummy"), 0);

  BOOST_REQUIRE_EQUAL (s.add_storage ("dummy"), -ENOTDIR);

  BOOST_REQUIRE_EQUAL (s.add_storage ("sub1"), 0);
  {
    FileStorage *sub = s.get_storage ("sub1");
    BOOST_REQUIRE (sub);
    sub->save("dummy", "dummy");
    std::string dummy;
    sub->load("dummy", dummy);
    sub->remove("dummy");
    BOOST_REQUIRE_EQUAL (dummy, "dummy");
  }
  BOOST_REQUIRE_EQUAL (s.del_storage("sub1"), 0);
}

void test_complex_state ()
{
  using namespace fhg::plugin::core;
  FileStorage s (working_dir);

  {
    std::map<int, std::string> map;
    for (int i = 0; i < 10; ++i)
      map[i] = boost::lexical_cast<std::string>(i);
    s.save ("map", map);
  }

  {
    std::map<int, std::string> map;
    s.load ("map", map);
    for (int i = 0; i < 10; ++i)
    {
      BOOST_REQUIRE_EQUAL (i, boost::lexical_cast<int>(map[i]));
    }
  }
}

test_suite*
init_unit_test_suite (int ac, char *av[])
{
  if (ac <= 1)
  {
    throw boost::unit_test::framework::setup_error
      ("I need the path to a temporary dictory to work in!");
  }

  working_dir  = fs::path(av[1]);
  working_dir /= "store";

  {
    test_suite* ts1 = BOOST_TEST_SUITE( "basic_storage" );
    ts1->add( BOOST_TEST_CASE(&test_save_load_delete));
    ts1->add( BOOST_TEST_CASE(&test_restore));
    ts1->add( BOOST_TEST_CASE(&test_hierachy));
    ts1->add( BOOST_TEST_CASE(&test_complex_state));
    ts1->add( BOOST_TEST_CASE(&test_save_load_invalid));

    framework::master_test_suite().add( ts1 );
  }

  return 0;
}
