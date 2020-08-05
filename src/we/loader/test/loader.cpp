#include <boost/test/unit_test.hpp>

#include <we/loader/test/order/stack.hpp>

#include <we/loader/loader.hpp>
#include <we/loader/exceptions.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

BOOST_AUTO_TEST_CASE (answer_question)
{
  we::loader::loader loader ({"."});

  {
    expr::eval::context out;

    loader["answer"].call ( "answer", nullptr, expr::eval::context(), out
                          , std::map<std::string, void*>()
                          );

    BOOST_REQUIRE_EQUAL
      (out.value ({"out"}), pnet::type::value::value_type (42L));
  }

  {
    expr::eval::context out;

    loader["question"].call ( "question", nullptr, expr::eval::context(), out
                            , std::map<std::string, void*>()
                            );

    BOOST_REQUIRE_EQUAL
      (out.value ({"out"}), pnet::type::value::value_type (44L));
    BOOST_REQUIRE_EQUAL
      (out.value ({"ans"}), pnet::type::value::value_type (42L));
  }

  {
    expr::eval::context out;

    loader["answer"].call ( "answer", nullptr, expr::eval::context(), out
                          , std::map<std::string, void*>()
                          );

    BOOST_REQUIRE_EQUAL
      (out.value ({"out"}), pnet::type::value::value_type (42L));
  }
}

BOOST_AUTO_TEST_CASE (bracket_not_found_empty_search_path)
{
  we::loader::loader loader ({});

  fhg::util::testing::require_exception
    ( [&loader] { loader["name"]; }
    , we::loader::module_not_found ("libname.so", "")
    );
}

BOOST_AUTO_TEST_CASE (bracket_not_found_nonempty_search_path)
{
  we::loader::loader loader ({"<p>","<q>"});

  fhg::util::testing::require_exception
    ( [&loader] { loader["name"]; }
    , we::loader::module_not_found ("libname.so", "\"<p>\":\"<q>\"")
    );
}

BOOST_AUTO_TEST_CASE (bracket_okay_load)
{
  we::loader::loader loader ({"."});

  // \note Test was checking the side effect of observing path, but
  // that accessor was removed, so now is testing it to not throw
  // only.
  loader["answer"];
}

BOOST_AUTO_TEST_CASE (load_order)
{
  we::loader::loader loader ({"."});

  // \note Test was checking the side effect of observing path, but
  // that accessor was removed, so now is testing it to not throw
  // only.
  loader["order_a"];
  loader["order_b"];

  BOOST_REQUIRE_EQUAL (start().size(), 2);
}
