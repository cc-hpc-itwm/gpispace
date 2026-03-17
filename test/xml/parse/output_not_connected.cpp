// Copyright (C) 2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/xml/parse/error.hpp>
#include <gspc/xml/parse/parser.hpp>
#include <gspc/xml/parse/state.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <fmt/format.h>
#include <sstream>
#include <string>

BOOST_AUTO_TEST_CASE (output_port_not_connected_in_expression)
{
  std::string const transition_name
    (gspc::testing::random_identifier_without_leading_underscore());
  std::string const port_name
    (gspc::testing::random_identifier_without_leading_underscore());

  std::string const input
    ( fmt::format (R"EOS(<defun name="foo">
 <net>
  <transition name="{0}">
    <defun>
      <out name="{1}" type="int"/>
      <expression/>
    </defun>
  </transition>
 </net>
</defun>)EOS"
      , transition_name
      , port_name
      )
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::output_not_connected>
    ( [&input]
      {
        gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
        ( "ERROR: output port {} is not connected"
          " in transition {} at {}"
        , port_name
        , transition_name
        , "[<stdin>:5:7]"
        )
    );
}

BOOST_AUTO_TEST_CASE (output_port_not_connected_in_module)
{
  std::string const transition_name
    (gspc::testing::random_identifier_without_leading_underscore());
  std::string const port_name
    (gspc::testing::random_identifier_without_leading_underscore());

  std::string const input
    ( fmt::format (R"EOS(<defun name="foo">
 <net>
  <transition name="{0}">
    <defun>
      <out name="{1}" type="int"/>
      <module name="m" function="f ({1})">
        <code></code>
      </module>
    </defun>
  </transition>
 </net>
</defun>)EOS"
      , transition_name
      , port_name
      )
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::output_not_connected>
    ( [&input]
      {
        gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
        ( "ERROR: output port {} is not connected"
          " in transition {} at {}"
        , port_name
        , transition_name
        , "[<stdin>:5:7]"
        )
    );
}

BOOST_AUTO_TEST_CASE (multiple_output_ports_one_not_connected)
{
  std::string const transition_name
    (gspc::testing::random_identifier_without_leading_underscore());
  std::string const connected_port
    (gspc::testing::random_identifier_without_leading_underscore());
  std::string const unconnected_port
    (gspc::testing::random_identifier_without_leading_underscore());

  std::string const input
    ( fmt::format (R"EOS(<defun name="foo">
 <net>
  <transition name="{0}">
    <defun>
      <out name="{1}" type="int"/>
      <out name="{2}" type="int"/>
      <expression/>
    </defun>
    <connect-out port="{1}" place="out"/>
  </transition>
  <place name="out" type="int"/>
 </net>
</defun>)EOS"
      , transition_name
      , connected_port
      , unconnected_port
      )
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::output_not_connected>
    ( [&input]
      {
        gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
        ( "ERROR: output port {} is not connected"
          " in transition {} at {}"
        , unconnected_port
        , transition_name
        , "[<stdin>:6:7]"
        )
    );
}
