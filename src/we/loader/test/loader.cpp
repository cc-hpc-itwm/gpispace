// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE loader
#include <boost/test/unit_test.hpp>

#include <we/loader/test/order/stack.hpp>

#include <we/loader/loader.hpp>
#include <we/loader/exceptions.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>
#include <fhg/util/boost/test/require_exception.hpp>

BOOST_AUTO_TEST_CASE (answer_question)
{
  we::loader::loader loader ({"."});

  {
    expr::eval::context out;

    loader["answer"].call ( "answer", nullptr, expr::eval::context(), out
                          , std::map<std::string, void*>()
                          );

    BOOST_REQUIRE_EQUAL
      (out.value ("out"), pnet::type::value::value_type (42L));
  }

  {
    expr::eval::context out;

    loader["question"].call ( "question", nullptr, expr::eval::context(), out
                            , std::map<std::string, void*>()
                            );

    BOOST_REQUIRE_EQUAL
      (out.value ("out"), pnet::type::value::value_type (44L));
    BOOST_REQUIRE_EQUAL
      (out.value ("ans"), pnet::type::value::value_type (42L));
  }

  {
    expr::eval::context out;

    loader["answer"].call ( "answer", nullptr, expr::eval::context(), out
                          , std::map<std::string, void*>()
                          );

    BOOST_REQUIRE_EQUAL
      (out.value ("out"), pnet::type::value::value_type (42L));
  }
}

BOOST_AUTO_TEST_CASE (bracket_not_found_empty_search_path)
{
  we::loader::loader loader ({});

  fhg::util::boost::test::require_exception<we::loader::module_not_found>
    ( [&loader] { loader["name"]; }
    , "module 'libname.so' not found in ''"
    );
}

BOOST_AUTO_TEST_CASE (bracket_not_found_nonempty_search_path)
{
  we::loader::loader loader ({"<p>","<q>"});

  fhg::util::boost::test::require_exception<we::loader::module_not_found>
    ( [&loader] { loader["name"]; }
    , "module 'libname.so' not found in '\"<p>\":\"<q>\"'"
    );
}

BOOST_AUTO_TEST_CASE (bracket_okay_load)
{
  we::loader::loader loader ({"."});

  BOOST_REQUIRE_EQUAL (loader["answer"].path(), "./libanswer.so");
}

BOOST_AUTO_TEST_CASE (load_order)
{
  we::loader::loader loader ({"."});

  BOOST_REQUIRE_EQUAL (loader["order_a"].path(), "./liborder_a.so");
  BOOST_REQUIRE_EQUAL (loader["order_b"].path(), "./liborder_b.so");

  BOOST_REQUIRE_EQUAL (start().size(), 2);
}
