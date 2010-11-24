#define BOOST_TEST_MODULE UtilThreadEventTest
#include <boost/test/unit_test.hpp>

#include <fhg/util/ini-parser.hpp>

#include <boost/unordered_map.hpp>
#include <boost/bind.hpp>

static int dummy_handler( std::string const &
                        , std::string const *
                        , std::string const &
                        , std::string const &
                        )
{
  return 0;
}

struct parse_into_map_t
{
  typedef boost::unordered_map<std::string, std::string> entries_t;

  int operator () ( std::string const & sec
                  , std::string const * secid
                  , std::string const & key
                  , std::string const & val
                  )
  {
    return handle (sec,secid,key,val);
  }

  int handle ( std::string const & sec
             , std::string const * secid
             , std::string const & key
             , std::string const & val
             )
  {
    std::string k (secid ? (sec + "." + *secid) : sec);
    k += ".";
    k += key;
    entries[k] = val;
    return 0;
  }

  std::string get ( std::string const & sec
                  , std::string const & key
                  , std::string const & def
                  )
  {
    try
    {
      std::string k (sec + "." + key);
      return entries.at(k);
    }
    catch (std::exception const &)
    {
      return def;
    }
  }

  std::string get ( std::string const & sec
                  , std::string const & sec_id
                  , std::string const & key
                  , std::string const & def
                  )
  {
    return get (sec + "." + sec_id, key, def);
  }

  entries_t entries;
};

BOOST_AUTO_TEST_CASE ( parse_no_such_file )
{
  try
  {
    fhg::util::ini::parse ("/this/file/does/not/exist", &dummy_handler);
  }
  catch (fhg::util::ini::exception::parse_error const & pe)
  {
    BOOST_CHECK (true);
  }
}

BOOST_AUTO_TEST_CASE ( parse_key_value_with_spaces )
{
  parse_into_map_t m;
  std::stringstream sstr ("foo = bar");
  fhg::util::ini::parse (sstr, boost::ref(m));
  BOOST_CHECK_EQUAL (m.get("", "foo", "baz"), "bar");
}

BOOST_AUTO_TEST_CASE ( parse_key_value_without_spaces )
{
  parse_into_map_t m;
  std::stringstream sstr ("foo=bar");
  fhg::util::ini::parse (sstr, boost::ref(m));
  BOOST_CHECK_EQUAL (m.get("", "foo", "baz"), "bar");
}

BOOST_AUTO_TEST_CASE ( parse_section_key_value )
{
  parse_into_map_t m;
  std::stringstream sstr ("[test]\nfoo=bar");
  fhg::util::ini::parse (sstr, boost::ref(m));
  BOOST_CHECK_EQUAL (m.get("test", "foo", "baz"), "bar");
}

BOOST_AUTO_TEST_CASE ( parse_section_key_value_with_space )
{
  parse_into_map_t m;
  std::stringstream sstr ("[ test   ]\nfoo=bar");
  fhg::util::ini::parse (sstr, boost::ref(m));
  BOOST_CHECK_EQUAL (m.get("test", "foo", "baz"), "bar");
}

BOOST_AUTO_TEST_CASE ( parse_section_with_empty_id )
{
  parse_into_map_t m;
  std::stringstream sstr ("[test \"\"]\nfoo=bar");
  fhg::util::ini::parse (sstr, boost::ref(m));
  BOOST_CHECK_EQUAL (m.get("test", "", "foo", "baz"), "bar");
}

BOOST_AUTO_TEST_CASE ( parse_section_with_id )
{
  parse_into_map_t m;
  std::stringstream sstr ("[test \"id\"]\nfoo=bar");
  fhg::util::ini::parse (sstr, boost::ref(m));
  BOOST_CHECK_EQUAL (m.get("test", "id", "foo", "baz"), "bar");
}

BOOST_AUTO_TEST_CASE ( parse_section_with_dot_id )
{
  parse_into_map_t m;
  std::stringstream sstr ("[test \"id.1\"]\nfoo=bar");
  fhg::util::ini::parse (sstr, boost::ref(m));
  BOOST_CHECK_EQUAL (m.get("test", "id.1", "foo", "baz"), "bar");
}

BOOST_AUTO_TEST_CASE ( parse_comment )
{
  parse_into_map_t m;
  std::stringstream sstr ("; comment 1\n# comment 2");
  fhg::util::ini::parse (sstr, boost::ref(m));
  BOOST_CHECK_EQUAL (m.get("test", "foo", ""), "");
}

BOOST_AUTO_TEST_CASE ( parse_include )
{
  parse_into_map_t m;
  std::stringstream sstr ("%include <file>");
  fhg::util::ini::parse (sstr, boost::ref(m));
}

BOOST_AUTO_TEST_CASE ( parse_invalid_line )
{
  parse_into_map_t m;
  std::stringstream sstr ("illegal line here");
  try
  {
    fhg::util::ini::parse (sstr, boost::ref(m));
    BOOST_CHECK(false);
  }
  catch (fhg::util::ini::exception::parse_error const &)
  {
    // ok
  }
}
