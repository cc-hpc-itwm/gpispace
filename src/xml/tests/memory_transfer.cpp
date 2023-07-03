// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/type/memory_transfer.hpp>

#include <fhg/util/xml.hpp>
#include <util-generic/join.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random/string.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/format.hpp>
#include <boost/test/data/test_case.hpp>

#include <functional>
#include <iomanip>

namespace
{
  template<typename Transfer>
  void check_dump ( std::size_t num_expressions_global
                  , std::size_t num_expressions_local
                  , std::string const& tag
                  , std::function<Transfer ( std::string const&
                                           , std::string const&
                                           )> make_transfer
                  , ::boost::optional<bool> not_modified_in_module_call
                  , ::boost::optional<bool> allow_empty_ranges
                  )
  {
    std::list<std::string> expressions_global;

    while (num_expressions_global --> 0)
    {
      expressions_global.push_back (fhg::util::testing::random_string_without_zero());
    }

    std::list<std::string> expressions_local;

    while (num_expressions_local --> 0)
    {
      expressions_local.push_back (fhg::util::testing::random_string_without_zero());
    }

    std::ostringstream oss;

    fhg::util::xml::xmlstream s (oss);

    xml::parse::type::dump::dump
      (s, make_transfer ( fhg::util::join (expressions_global, ';').string()
                        , fhg::util::join (expressions_local, ';').string()
                        )
      );

    const std::string expected
      ( ( ::boost::format (R"EOS(<memory-%1%%4%%5%>
  <global>%2%</global>
  <local>%3%</local>
</memory-%1%>)EOS")
        % tag
        % fhg::util::join (expressions_global, ';')
        % fhg::util::join (expressions_local, ';')
        % ( not_modified_in_module_call
          ? ( ::boost::format (" not-modified-in-module-call=\"%1%\"")
            % (*not_modified_in_module_call ? "true" : "false")
            ).str()
          : ""
          )
        % ( allow_empty_ranges
          ? ( ::boost::format (" allow-empty-ranges=\"%1%\"")
            % (*allow_empty_ranges ? "true" : "false")
            ).str()
          : ""
          )
        ).str()
      );

    BOOST_REQUIRE_EQUAL (expected, oss.str());
  }

  std::vector<std::size_t> const counts ({0, 1, 2, 3});
  std::vector<::boost::optional<bool>> const tribool_values
    ({{false, false}, {true, false}, {true, true}});
}

BOOST_DATA_TEST_CASE
  ( dump_get
  , counts * counts * tribool_values
  , num_expressions_global
  , num_expressions_local
  , allow_empty_ranges
  )
{
  check_dump<xml::parse::type::memory_get>
    ( num_expressions_global
    , num_expressions_local
    , "get"
    , [&allow_empty_ranges]
      ( std::string const& expressions_global
      , std::string const& expressions_local
      )
      {
        return xml::parse::type::memory_get
          ( xml::parse::util::position_type
              (nullptr, nullptr, fhg::util::testing::random_string())
          , expressions_global
          , expressions_local
          , we::type::property::type()
          , allow_empty_ranges
          );
      }
    , ::boost::none
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
  check_dump<xml::parse::type::memory_put>
    ( num_expressions_global
    , num_expressions_local
    , "put"
    , [&not_modified_in_module_call, &allow_empty_ranges]
        ( std::string const& expressions_global
        , std::string const& expressions_local
        )
      {
        return xml::parse::type::memory_put
          ( xml::parse::util::position_type
              (nullptr, nullptr, fhg::util::testing::random_string())
          , expressions_global
          , expressions_local
          , we::type::property::type()
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
  check_dump<xml::parse::type::memory_getput>
    ( num_expressions_global
    , num_expressions_local
    , "getput"
    , [&not_modified_in_module_call, &allow_empty_ranges]
        ( std::string const& expressions_global
        , std::string const& expressions_local
        )
      {
        return xml::parse::type::memory_getput
          ( xml::parse::util::position_type
              (nullptr, nullptr, fhg::util::testing::random_string())
          , expressions_global
          , expressions_local
          , we::type::property::type()
          , not_modified_in_module_call
          , allow_empty_ranges
          );
      }
    , not_modified_in_module_call
    , allow_empty_ranges
    );
}
