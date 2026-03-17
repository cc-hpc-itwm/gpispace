// Copyright (C) 2010,2013-2016,2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <test/we/loader/order/stack.hpp>

#include <gspc/we/loader/exceptions.hpp>
#include <gspc/we/loader/loader.hpp>

#include <gspc/testing/printer/we/type/value.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/require_exception.hpp>

BOOST_AUTO_TEST_CASE (answer_question)
{
  gspc::we::loader::loader loader ({"."});

  {
    gspc::we::expr::eval::context out;

    loader["answer"].call ( "answer", nullptr, gspc::we::expr::eval::context(), out
                          , std::map<std::string, void*>()
                          );

    BOOST_REQUIRE_EQUAL
      (out.value ({"out"}), gspc::pnet::type::value::value_type (42L));
  }

  {
    gspc::we::expr::eval::context out;

    loader["question"].call ( "question", nullptr, gspc::we::expr::eval::context(), out
                            , std::map<std::string, void*>()
                            );

    BOOST_REQUIRE_EQUAL
      (out.value ({"out"}), gspc::pnet::type::value::value_type (44L));
    BOOST_REQUIRE_EQUAL
      (out.value ({"ans"}), gspc::pnet::type::value::value_type (42L));
  }

  {
    gspc::we::expr::eval::context out;

    loader["answer"].call ( "answer", nullptr, gspc::we::expr::eval::context(), out
                          , std::map<std::string, void*>()
                          );

    BOOST_REQUIRE_EQUAL
      (out.value ({"out"}), gspc::pnet::type::value::value_type (42L));
  }
}

BOOST_AUTO_TEST_CASE (bracket_not_found_empty_search_path)
{
  gspc::we::loader::loader loader ({});

  gspc::testing::require_exception
    ( [&loader] { loader["name"]; }
    , gspc::we::loader::module_not_found ("libname.so", "")
    );
}

BOOST_AUTO_TEST_CASE (bracket_not_found_nonempty_search_path)
{
  gspc::we::loader::loader loader ({"<p>","<q>"});

  gspc::testing::require_exception
    ( [&loader] { loader["name"]; }
    , gspc::we::loader::module_not_found ("libname.so", R"("<p>":"<q>")")
    );
}

BOOST_AUTO_TEST_CASE (bracket_okay_load)
{
  gspc::we::loader::loader loader ({"."});

  // \note Test was checking the side effect of observing path, but
  // that accessor was removed, so now is testing it to not throw
  // only.
  loader["answer"];
}

BOOST_AUTO_TEST_CASE (load_order)
{
  gspc::we::loader::loader loader ({"."});

  // \note Test was checking the side effect of observing path, but
  // that accessor was removed, so now is testing it to not throw
  // only.
  loader["order_a"];
  loader["order_b"];

  BOOST_REQUIRE_EQUAL (start().size(), 2);
}
