// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE loader
#include <boost/test/unit_test.hpp>

#include "answer.hpp"

#include <we/loader/loader.hpp>

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

BOOST_AUTO_TEST_CASE (load_not_found)
{
  we::loader::loader loader;

  fhg::util::boost::test::require_exception<we::loader::ModuleLoadFailed>
    ( boost::bind (&we::loader::loader::load, &loader, "name", "<path>")
    , "we::loader::ModuleLoadFailed"
    , "could not load module 'name' from '<path>': <path>:"
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

  fhg::util::boost::test::require_exception<we::loader::ModuleLoadFailed>
    ( boost::bind (&we::loader::loader::load, &loader, "name", "<path>")
    , "we::loader::ModuleLoadFailed"
    , "module 'name' already registered"
    );
}
