#define BOOST_TEST_MODULE pnete_ui_graph_style_cached_predicates

#include <pnete/ui/graph/style/store.hpp>

#include <boost/bind.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>

typedef std::string value_type;
typedef fhg::pnete::ui::graph::style::store::cached_predicates<std::string, value_type> store_type;
typedef store_type::optional_value_type optional_value_type;

std::ostream& operator << ( std::ostream& s
                          , const optional_value_type& v
                          )
{
  if (v)
    {
      s << "Just " << *v;
    }
  else
    {
      s << "Nothing";
    }

  return s;
}

static std::size_t count_eval;

optional_value_type beep_if ( const std::string& what
                            , const std::string& key
                            )
{
  static const std::string b ("beep");

  std::cerr << "EVAL: beep_if (" << what << ", " << key << ")" << std::endl;
  ++count_eval;

  if (what == key)
    {
      return optional_value_type (boost::ref (b));
    }

  return boost::none;
}



optional_value_type short_id ( const std::size_t& len
                             , const std::string& key
                             )
{
  static std::string s;

  std::cerr << "EVAL: short_id (" << len << ", " << key << ")" << std::endl;
  ++count_eval;

  if (key.size() < len)
    {
      s = key;

      return optional_value_type (boost::ref (s));
    }

  return boost::none;
}

optional_value_type fallback (const std::string&)
{
  static const std::string f ("fallback");

  return optional_value_type (boost::ref (f));
}

#define REQUIRE(b) \
  if (!(b)) { std::cerr << "FAILURE in line " << __LINE__ << std::endl; \
              exit (EXIT_FAILURE); \
            }

optional_value_type get ( const store_type& store
                        , const std::string& key
                        )
{
  count_eval = 0;
  const optional_value_type r (store.get (key));
  std::cout << r << std::endl;
  return r;
}

namespace
{
  optional_value_type opt (const std::string& value)
  {
    return boost::make_optional (value);
  }
}

BOOST_AUTO_TEST_CASE (all)
{
  store_type store;

  store.push (boost::bind (&beep_if, std::string ("what"), _1));
  store.push (boost::bind (&short_id, 4, _1));

  {
    const optional_value_type res (get (store, "what"));
    BOOST_REQUIRE_EQUAL (count_eval, 1UL);
    BOOST_REQUIRE_EQUAL (res, opt ("beep"));
  }
  {
    const optional_value_type res (get (store, "id"));
    BOOST_REQUIRE_EQUAL (count_eval, 2UL);
    BOOST_REQUIRE_EQUAL (res, opt ("id"));
  }
  {
    const optional_value_type res (get (store, "idd"));
    BOOST_REQUIRE_EQUAL (count_eval, 2UL);
    BOOST_REQUIRE_EQUAL (res, opt ("idd"));
  }
  {
    const optional_value_type res (get (store, "to long"));
    BOOST_REQUIRE_EQUAL (count_eval, 2UL);
    BOOST_REQUIRE_EQUAL (res, boost::none);
  }

  {
    const optional_value_type res (get (store, "what"));
    BOOST_REQUIRE_EQUAL (count_eval, 0UL);
    BOOST_REQUIRE_EQUAL (res, opt ("beep"));
  }
  {
    const optional_value_type res (get (store, "id"));
    BOOST_REQUIRE_EQUAL (count_eval, 0UL);
    // yes, because its the same ref but no re-eval!
    BOOST_REQUIRE_EQUAL (res, opt ("idd"));
  }
  {
    const optional_value_type res (get (store, "idd"));
    BOOST_REQUIRE_EQUAL (count_eval, 0UL);
    BOOST_REQUIRE_EQUAL (res, opt ("idd"));
  }
  {
    const optional_value_type res (get (store, "to long"));
    BOOST_REQUIRE_EQUAL (count_eval, 0UL);
    BOOST_REQUIRE_EQUAL (res, boost::none);
  }
}


BOOST_AUTO_TEST_CASE (fallback_if_non_existing)
{
  store_type store;

  store.push (boost::bind (&beep_if, std::string ("what"), _1));
  store.push (boost::bind (&short_id, 4, _1));
  store.push (&fallback);

  {
    const optional_value_type res (get (store, "what"));
    BOOST_REQUIRE_EQUAL (count_eval, 1UL);
    BOOST_REQUIRE_EQUAL (res, opt ("beep"));
  }
  {
    const optional_value_type res (get (store, "id"));
    BOOST_REQUIRE_EQUAL (count_eval, 2UL);
    BOOST_REQUIRE_EQUAL (res, opt ("id"));
  }
  {
    const optional_value_type res (get (store, "idd"));
    BOOST_REQUIRE_EQUAL (count_eval, 2UL);
    BOOST_REQUIRE_EQUAL (res, opt ("idd"));
  }
  {
    const optional_value_type res (get (store, "to long"));
    BOOST_REQUIRE_EQUAL (count_eval, 2UL);
    BOOST_REQUIRE_EQUAL (res, opt ("fallback"));
  }

  {
    const optional_value_type res (get (store, "what"));
    BOOST_REQUIRE_EQUAL (count_eval, 0UL);
    BOOST_REQUIRE_EQUAL (res, opt ("beep"));
  }
  {
    const optional_value_type res (get (store, "id"));
    BOOST_REQUIRE_EQUAL (count_eval, 0UL);
    BOOST_REQUIRE_EQUAL (res, opt ("idd"));
  }
  {
    const optional_value_type res (get (store, "idd"));
    BOOST_REQUIRE_EQUAL (count_eval, 0UL);
    // yes, because its the same ref but no re-eval!
    BOOST_REQUIRE_EQUAL (res, opt ("idd"));
  }
  {
    const optional_value_type res (get (store, "to long"));
    BOOST_REQUIRE_EQUAL (count_eval, 0UL);
    BOOST_REQUIRE_EQUAL (res, opt ("fallback"));
  }
}
