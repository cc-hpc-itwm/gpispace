#define BOOST_TEST_MODULE pnete_ui_graph_style_cached_predicates

#include <pnete/ui/graph/style/store.hpp>

#include <boost/bind.hpp>
#include <boost/test/unit_test.hpp>

typedef fhg::pnete::ui::graph::style::store::cached_predicates<std::string, std::string> store_type;

namespace
{
  boost::optional<std::string> beep_if
    (const std::string& what, size_t* counter, const std::string& key)
  {
    ++(*counter);
    return boost::make_optional (what == key, std::string ("beep"));
  }

  boost::optional<std::string> fallback (const std::string&)
  {
    return std::string ("fallback");
  }

  boost::optional<std::string> opt (const std::string& value)
  {
    return boost::make_optional (value);
  }
}

BOOST_AUTO_TEST_CASE (caching)
{
  store_type store;
  size_t count_eval (0);

  store.push (boost::bind (&beep_if, std::string ("what"), &count_eval, _1));

  BOOST_REQUIRE_EQUAL (count_eval, 0UL);

  BOOST_REQUIRE_EQUAL (store.get ("what"), opt ("beep"));
  BOOST_REQUIRE_EQUAL (count_eval, 1UL);

  BOOST_REQUIRE_EQUAL (store.get ("unknown"), boost::none);
  BOOST_REQUIRE_EQUAL (count_eval, 2UL);

  count_eval = 0UL;

  BOOST_REQUIRE_EQUAL (store.get ("what"), opt ("beep"));
  BOOST_REQUIRE_EQUAL (count_eval, 0UL);

  BOOST_REQUIRE_EQUAL (store.get ("unknown"), boost::none);
  BOOST_REQUIRE_EQUAL (count_eval, 0UL);

  store.clear_cache();

  BOOST_REQUIRE_EQUAL (store.get ("what"), opt ("beep"));
  BOOST_REQUIRE_EQUAL (count_eval, 1UL);

  BOOST_REQUIRE_EQUAL (store.get ("unknown"), boost::none);
  BOOST_REQUIRE_EQUAL (count_eval, 2UL);
}

BOOST_AUTO_TEST_CASE (fallback_if_non_existing)
{
  store_type store;
  size_t count_eval (0);

  store.push (boost::bind (&beep_if, std::string ("what"), &count_eval, _1));
  store.push (fallback);

  BOOST_REQUIRE_EQUAL (store.get ("what"), opt ("beep"));
  BOOST_REQUIRE_EQUAL (store.get ("unknown"), opt ("fallback"));
}
