// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE loader
#include <boost/test/unit_test.hpp>

#include "order/stack.hpp"

#include <we/loader/loader.hpp>
#include <we/loader/exceptions.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/test/require_exception.hpp>

BOOST_AUTO_TEST_CASE (fresh_has_empty_search_path)
{
  we::loader::loader loader;

  BOOST_REQUIRE_EQUAL (loader.search_path(), "");
}

BOOST_AUTO_TEST_CASE (append_and_clear_serch_path)
{
  we::loader::loader loader;
  loader.append_search_path ("p");

  BOOST_REQUIRE_EQUAL (loader.search_path(), "\"p\"");

  loader.append_search_path ("q");

  BOOST_REQUIRE_EQUAL (loader.search_path(), "\"p\":\"q\"");

  loader.clear_search_path();

  BOOST_REQUIRE_EQUAL (loader.search_path(), "");
}

BOOST_AUTO_TEST_CASE (answer_question)
{
  we::loader::loader loader;

  loader.append_search_path (".");

  {
    expr::eval::context out;

    loader["answer"].call ("answer", 0, expr::eval::context(), out);

    BOOST_REQUIRE_EQUAL
      (out.value ("out"), pnet::type::value::value_type (42L));
  }

  {
    expr::eval::context out;

    loader["question"].call ("question", 0, expr::eval::context(), out);

    BOOST_REQUIRE_EQUAL
      (out.value ("out"), pnet::type::value::value_type (44L));
    BOOST_REQUIRE_EQUAL
      (out.value ("ans"), pnet::type::value::value_type (42L));
  }

  {
    expr::eval::context out;

    loader["answer"].call ("answer", 0, expr::eval::context(), out);

    BOOST_REQUIRE_EQUAL
      (out.value ("out"), pnet::type::value::value_type (42L));
  }
}

BOOST_AUTO_TEST_CASE (load_failed)
{
  we::loader::loader loader;

  fhg::util::boost::test::require_exception<we::loader::module_load_failed>
    ( boost::bind (&we::loader::loader::load, &loader, "name", "<path>")
    , "could not load module '<path>': <path>:"
      " cannot open shared object file: No such file or directory"
    );
}

BOOST_AUTO_TEST_CASE (load_okay)
{
  we::loader::loader loader;

  BOOST_REQUIRE (loader.load ("name", "./libanswer.so"));
}

BOOST_AUTO_TEST_CASE (load_already_registered)
{
  we::loader::loader loader;

  BOOST_REQUIRE (loader.load ("name", "./libanswer.so"));

  fhg::util::boost::test::require_exception<we::loader::module_already_registered>
    ( boost::bind (&we::loader::loader::load, &loader, "name", "<path>")
    , "module 'name' already registered"
    );
}

BOOST_AUTO_TEST_CASE (bracket_not_found)
{
  we::loader::loader loader;

  fhg::util::boost::test::require_exception<we::loader::module_not_found>
    ( boost::bind (&we::loader::loader::operator[], &loader, "name")
    , "module 'libname.so' not found in ''"
    );

  loader.append_search_path ("<p>");
  loader.append_search_path ("<q>");

  fhg::util::boost::test::require_exception<we::loader::module_not_found>
    ( boost::bind (&we::loader::loader::operator[], &loader, "name")
    , "module 'libname.so' not found in '\"<p>\":\"<q>\"'"
    );
}

BOOST_AUTO_TEST_CASE (bracket_okay_load)
{
  we::loader::loader loader;
  loader.append_search_path (".");

  BOOST_REQUIRE_EQUAL (loader["answer"].name(), "answer");
}

BOOST_AUTO_TEST_CASE (bracket_okay_from_table)
{
  we::loader::loader loader;
  loader.append_search_path (".");

  BOOST_REQUIRE_EQUAL (loader["answer"].name(), "answer");

  loader.clear_search_path();

  BOOST_REQUIRE_EQUAL (loader["answer"].name(), "answer");
}

BOOST_AUTO_TEST_CASE (unload_order)
{
  {
    we::loader::loader loader;

    BOOST_REQUIRE (loader.load ("a", "./liborder_a.so"));
    BOOST_REQUIRE (loader.load ("b", "./liborder_b.so"));

    BOOST_REQUIRE_EQUAL (start().size(), 2);
  }

  BOOST_REQUIRE_EQUAL (stop().size(), 2);

  std::stack<std::string> stop_expected;
  while (!start().empty())
  {
    stop_expected.push (start().top()); start().pop();
  }

  while (!stop().empty())
  {
    BOOST_REQUIRE_EQUAL (stop().top(), stop_expected.top());
    stop().pop();
    stop_expected.pop();
  }
}
