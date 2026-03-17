// Copyright (C) 2014-2016,2018,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/memory_transfer.hpp>

#include <gspc/util/xml.hpp>
#include <gspc/util/join.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/optional.hpp>
#include <gspc/testing/random/string.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <gspc/util/join.formatter.hpp>
#include <fmt/core.h>
#include <functional>
#include <iomanip>
#include <optional>

namespace
{
  template<typename Transfer>
  void check_dump ( std::size_t num_expressions_global
                  , std::size_t num_expressions_local
                  , std::string const& tag
                  , std::function<Transfer ( std::string const&
                                           , std::string const&
                                           )> make_transfer
                  , std::optional<bool> not_modified_in_module_call
                  , std::optional<bool> allow_empty_ranges
                  )
  {
    std::list<std::string> expressions_global;

    while (num_expressions_global --> 0)
    {
      expressions_global.push_back (gspc::testing::random_string_without_zero());
    }

    std::list<std::string> expressions_local;

    while (num_expressions_local --> 0)
    {
      expressions_local.push_back (gspc::testing::random_string_without_zero());
    }

    std::ostringstream oss;

    gspc::util::xml::xmlstream s (oss);

    gspc::xml::parse::type::dump::dump
      (s, make_transfer ( gspc::util::join (expressions_global, ';').string()
                        , gspc::util::join (expressions_local, ';').string()
                        )
      );

    const std::string expected
      ( fmt::format (R"EOS(<memory-{0}{3}{4}>
  <global>{1}</global>
  <local>{2}</local>
</memory-{0}>)EOS"
        , tag
        , gspc::util::join (expressions_global, ';')
        , gspc::util::join (expressions_local, ';')
        , ( not_modified_in_module_call
          ? fmt::format ( " not-modified-in-module-call=\"{}\""
                        , *not_modified_in_module_call ? "true" : "false"
                        )
          : ""
          )
        , ( allow_empty_ranges
          ? fmt::format ( " allow-empty-ranges=\"{}\""
                        , *allow_empty_ranges ? "true" : "false"
                        )
          : ""
          )
        )
      );

    BOOST_REQUIRE_EQUAL (expected, oss.str());
  }

  std::vector<std::size_t> const counts ({0, 1, 2, 3});
  std::vector<std::optional<bool>> const tribool_values
    ({std::nullopt, std::optional{false}, std::optional{true}});
}

BOOST_DATA_TEST_CASE
  ( dump_get
  , counts * counts * tribool_values
  , num_expressions_global
  , num_expressions_local
  , allow_empty_ranges
  )
{
  check_dump<gspc::xml::parse::type::memory_get>
    ( num_expressions_global
    , num_expressions_local
    , "get"
    , [&allow_empty_ranges]
      ( std::string const& expressions_global
      , std::string const& expressions_local
      )
      {
        return gspc::xml::parse::type::memory_get
          ( gspc::xml::parse::util::position_type
              (nullptr, nullptr, gspc::testing::random_string())
          , expressions_global
          , expressions_local
          , gspc::we::type::property::type()
          , allow_empty_ranges
          );
      }
    , std::nullopt
    , allow_empty_ranges
    );
}

BOOST_DATA_TEST_CASE
  ( dump_put
  , counts * counts * tribool_values * tribool_values
  , num_expressions_global
  , num_expressions_local
  , not_modified_in_module_call
  , allow_empty_ranges
  )
{
  check_dump<gspc::xml::parse::type::memory_put>
    ( num_expressions_global
    , num_expressions_local
    , "put"
    , [&not_modified_in_module_call, &allow_empty_ranges]
        ( std::string const& expressions_global
        , std::string const& expressions_local
        )
      {
        return gspc::xml::parse::type::memory_put
          ( gspc::xml::parse::util::position_type
              (nullptr, nullptr, gspc::testing::random_string())
          , expressions_global
          , expressions_local
          , gspc::we::type::property::type()
          , not_modified_in_module_call
          , allow_empty_ranges
          );
      }
    , not_modified_in_module_call
    , allow_empty_ranges
    );
}

BOOST_DATA_TEST_CASE
  ( dump_getput
  , counts * counts * tribool_values * tribool_values
  , num_expressions_global
  , num_expressions_local
  , not_modified_in_module_call
  , allow_empty_ranges
  )
{
  check_dump<gspc::xml::parse::type::memory_getput>
    ( num_expressions_global
    , num_expressions_local
    , "getput"
    , [&not_modified_in_module_call, &allow_empty_ranges]
        ( std::string const& expressions_global
        , std::string const& expressions_local
        )
      {
        return gspc::xml::parse::type::memory_getput
          ( gspc::xml::parse::util::position_type
              (nullptr, nullptr, gspc::testing::random_string())
          , expressions_global
          , expressions_local
          , gspc::we::type::property::type()
          , not_modified_in_module_call
          , allow_empty_ranges
          );
      }
    , not_modified_in_module_call
    , allow_empty_ranges
    );
}
