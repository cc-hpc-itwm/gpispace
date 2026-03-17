// Copyright (C) 2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests that the parser rejects an in-port and out-port of the
// same function that share a name but differ in type.
//
// The `port_type_mismatch` error is raised during parsing when
// a port name is used for both input and output directions with
// different type specifiers.  Additionally, the warning
// `conflicting_port_types` is issued during `xml_to_we`
// (inside `transition_synthesize`) when the resolved function
// of a transition has in/out ports that share a name but differ
// in signature.  The error is tested directly; the warning is
// tested indirectly through the negative case.

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <gspc/xml/parse/error.hpp>
#include <gspc/xml/parse/parser.hpp>

#include <gspc/we/type/net.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <fmt/core.h>

#include <sstream>
#include <string>
#include <vector>

namespace
{
  [[nodiscard]] auto all_types() -> std::vector<std::string>
  {
    return std::vector<std::string>
      { "control"
      , "bool"
      , "int"
      , "long"
      , "float"
      , "double"
      , "char"
      , "string"
      , "bitset"
      , "bytearray"
      , "map"
      , "set"
      , "list"
      };
  }

  [[nodiscard]] auto make_conflicting_input
    ( std::string const& input_type
    , std::string const& output_type
    ) -> std::string
  {
    return fmt::format
      ( "<defun name=\"port_type\">"
        "<in name=\"x\" type=\"{}\"/>"
        "<out name=\"x\" type=\"{}\"/>"
        "<expression/>"
        "</defun>"
      , input_type
      , output_type
      );
  }

  [[nodiscard]] auto make_non_conflicting_input
    (std::string const& type) -> std::string
  {
    return fmt::format
      ( "<defun name=\"top\">"
        "<net>"
        "<place name=\"p\" type=\"{}\"/>"
        "<transition name=\"t\">"
        "<defun>"
        "<in name=\"x\" type=\"{}\"/>"
        "<out name=\"x\" type=\"{}\"/>"
        "<expression/>"
        "</defun>"
        "<connect-in port=\"x\" place=\"p\"/>"
        "<connect-out port=\"x\" place=\"p\"/>"
        "</transition>"
        "</net>"
        "</defun>"
      , type
      , type
      , type
      );
  }
}

BOOST_DATA_TEST_CASE
  ( conflicting_port_types
  , boost::unit_test::data::make (all_types())
  * boost::unit_test::data::make (all_types())
  , input_type
  , output_type
  )
{
  if (input_type != output_type)
  {
    auto const input {make_conflicting_input (input_type, output_type)};

    auto state {gspc::xml::parse::state::type{}};
    auto input_stream {std::istringstream {input}};

    BOOST_CHECK_THROW
      ( gspc::xml::parse::just_parse (state, input_stream)
      , gspc::xml::parse::error::port_type_mismatch
      );
  }
  else
  {
    auto const input {make_non_conflicting_input (input_type)};

    auto state {gspc::xml::parse::state::type{}};
    state.warning_conflicting_port_types() = true;
    state.warning_error() = true;

    auto input_stream {std::istringstream {input}};
    auto function {gspc::xml::parse::just_parse (state, input_stream)};
    gspc::xml::parse::post_processing_passes (function, &state);

    BOOST_CHECK_NO_THROW (gspc::xml::parse::xml_to_we (function, state));
  }
}
