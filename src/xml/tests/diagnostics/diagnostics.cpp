// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/warning.hpp>

#include <parser_fixture.hpp>

#include <util-generic/executable_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <FMT/xml/parse/util/position.hpp>
#include <fmt/core.h>
#include <string>

namespace
{
  struct fixture : parser_fixture
  {
    template<typename Ex>
    void require_exception_from_generate_cpp
      (std::string const& expected_what)
    {
      ::boost::filesystem::path const path
        (fhg::util::executable_path().parent_path() / "gen");
      state.path_to_cpp() = path.string();

      fhg::util::testing::require_exception_with_message<Ex>
        ( [this]
          {
            ::xml::parse::generate_cpp (*function, state);
          }
        , expected_what
        );
    }

    template<typename Ex>
    void require_exception_from_parse
      (std::string const& expected_what)
    {
      fhg::util::testing::require_exception_with_message<Ex>
        ( [this]
          {
            parse();
          }
        , expected_what
        );
    }
  };
}

BOOST_AUTO_TEST_CASE (warning_struct_redefined)
{
  std::string const name_struct (fhg::util::testing::random_identifier());

  std::string const input
    ( fmt::format (R"EOS(
<defun name="{0}">
  <struct name="{1}"><field name="{2}" type="{3}"/></struct>
  <struct name="{1}"><field name="{2}" type="{3}"/></struct>
  <expression/>
</defun>)EOS"
      , fhg::util::testing::random_identifier()
      , name_struct
      , fhg::util::testing::random_identifier()
      , fhg::util::testing::random_identifier()
      )
    );

  fhg::util::testing::require_exception_with_message
    <xml::parse::warning::struct_redefined>
    ( [&input]()
      { xml::parse::state::type state;
        state.warning_struct_redefined() = true;
        state.warning_error() = true;
        std::istringstream input_stream (input);
        auto function (xml::parse::just_parse (state, input_stream));
        xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format ( "WARNING: struct {0} at {1} redefined at {2}"
                  , name_struct
                  , "[<stdin>:3:3]"
                  , "[<stdin>:4:3]"
                  )
    );
}

BOOST_AUTO_TEST_CASE (error_struct_redefined)
{
  std::string const name_struct (fhg::util::testing::random_identifier());

  std::string const input
    ( fmt::format (R"EOS(
<defun name="{0}">
  <struct name="{1}"><field name="a" type="{2}"/></struct>
  <struct name="{1}"><field name="b" type="{2}"/></struct>
  <expression/>
</defun>)EOS"
      , fhg::util::testing::random_identifier()
      , name_struct
      , fhg::util::testing::random_identifier()
      )
    );

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::struct_redefined>
    ( [&input]()
      { xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (xml::parse::just_parse (state, input_stream));
        xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format ( "ERROR: struct {0} at {1} redefined at {2}"
                  , name_struct
                  , "[<stdin>:3:3]"
                  , "[<stdin>:4:3]"
                  )
    );
}

BOOST_AUTO_TEST_CASE (warning_struct_field_redefined)
{
  std::string const name_field {fhg::util::testing::random_identifier()};

  std::string const input
    ( fmt::format (R"EOS(
<defun name="{0}">
  <struct name="{1}">
    <field name="{2}" type="{3}"/>
    <field name="{2}" type="{4}"/>
  </struct>
  <expression/>
</defun>
)EOS"
      , fhg::util::testing::random_identifier()
      , fhg::util::testing::random_identifier()
      , name_field
      , fhg::util::testing::random_identifier()
      , fhg::util::testing::random_identifier()
      )
    );

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::struct_field_redefined>
    ( [&input]()
      { xml::parse::state::type state;
        std::istringstream input_stream (input);
        xml::parse::just_parse (state, input_stream);
      }
    , fmt::format ( "ERROR: struct field '{0}' redefined in {1}"
                  , name_field
                  , "<stdin>"
                  )
    );
}

BOOST_FIXTURE_TEST_CASE (warning_duplicate_external_function, fixture)
{
  parse ("diagnostics/warning_duplicate_external_function.xpnet");

  state.warning_duplicate_external_function() = true;
  state.warning_error() = true;

  require_exception_from_generate_cpp
    <xml::parse::warning::duplicate_external_function>
    ( fmt::format ( "WARNING: the external function f in module m"
                    " has multiple occurences in {0} and {1}"
                  , xml::parse::util::position_type (nullptr, nullptr, xpnet, 10, 9)
                  , xml::parse::util::position_type (nullptr, nullptr, xpnet, 5, 9)
                  )
    );
}

BOOST_AUTO_TEST_CASE (error_connect_response_with_unknown_port)
{
  std::string const port (fhg::util::testing::random_identifier());

  std::string const input
    ( fmt::format (R"EOS(
<defun name="{0}">
  <net>
    <transition name="{1}">
      <defun>
        <expression/>
      </defun>
      <connect-response port="{2}" to="{3}"/>
    </transition>
  </net>
</defun>)EOS"
      , fhg::util::testing::random_identifier()
      , fhg::util::testing::random_identifier_without_leading_underscore()
      , port
      , fhg::util::testing::random_identifier()
      )
    );

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::unknown_port_in_connect_response>
    ( [&input]()
      { xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (xml::parse::just_parse (state, input_stream));
        xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
      ( "ERROR: connect-response from unknown output port '{0}' in {1}"
      , port
      , "[<stdin>:8:7]"
      )
    );
}

BOOST_AUTO_TEST_CASE (error_connect_response_with_unknown_to)
{
  std::string const to (fhg::util::testing::random_identifier());

  std::string const input
    ( fmt::format (R"EOS(
<defun name="{0}">
  <net>
    <transition name="{1}">
      <defun>
        <out name="{2}" type="string"/>
        <expression/>
      </defun>
      <connect-response port="{2}" to="{3}"/>
    </transition>
  </net>
</defun>)EOS"
      , fhg::util::testing::random_identifier()
      , fhg::util::testing::random_identifier_without_leading_underscore()
      , fhg::util::testing::random_identifier_without_leading_underscore()
      , to
      )
    );

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::unknown_to_in_connect_response>
    ( [&input]()
      { xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (xml::parse::just_parse (state, input_stream));
        xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format ( "ERROR: unknown input port '{0}' in attribute 'to'"
                    " of connect-response in {1}"
                  , to
                  , "[<stdin>:9:7]"
                  )
    );
}

BOOST_AUTO_TEST_CASE (error_connect_response_invalid_signature)
{
  std::string const to
    (fhg::util::testing::random_identifier_without_leading_underscore());
  std::string const name (fhg::util::testing::random_identifier());

  std::string const input
    ( fmt::format (R"EOS(
<defun name="{0}">
  <net>
    <struct name="{1}"/>
    <transition name="{2}">
      <defun>
        <in name="{3}" type="{1}"/>
        <out name="{4}" type="string"/>
        <expression/>
      </defun>
      <connect-response port="{4}" to="{3}"/>
    </transition>
  </net>
</defun>)EOS"
      , fhg::util::testing::random_identifier()
      , name
      , fhg::util::testing::random_identifier_without_leading_underscore()
      , to
      , fhg::util::testing::random_identifier_without_leading_underscore()
      )
    );

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::invalid_signature_in_connect_response>
    ( [&input]()
      { xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (xml::parse::just_parse (state, input_stream));
        xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
      ( "ERROR: invalid signature for response to port '{0}'."
        " The type '{1}' with the signature '{1} :: []' does not provide"
        " the field 'response_id :: string' in {2}"
      , to
      , name
      , "[<stdin>:11:7]"
      )
    );
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_external_function, fixture)
{
  parse ("diagnostics/error_duplicate_external_function.xpnet");

  require_exception_from_generate_cpp
    <xml::parse::error::duplicate_external_function>
    ( fmt::format ( "ERROR: duplicate external function f in module m"
                    " has conflicting definition at {0}"
                    ", earlier definition is at {1}"
                  , xml::parse::util::position_type (nullptr, nullptr, xpnet, 5, 9)
                  , xml::parse::util::position_type (nullptr, nullptr, xpnet, 10, 9)
                  )
    );
}

#define GENERIC_DUPLICATE_FROM_FILE(_file,_type,_msg,_le,_ce,_ll,_cl)   \
  set_parse_input (_file);                                              \
                                                                        \
  require_exception_from_parse<xml::parse::error::duplicate_ ## _type>  \
  ( fmt::format ( "ERROR: duplicate " _msg " at {0}"                \
                    ", earlier definition is at {1}"                    \
                  , xml::parse::util::position_type (nullptr, nullptr, xpnet, _ll, _cl)       \
                  , xml::parse::util::position_type (nullptr, nullptr, xpnet, _le, _ce)       \
                  )                                                     \
  )

BOOST_FIXTURE_TEST_CASE (error_duplicate_connect_in_read, fixture)
{
  GENERIC_DUPLICATE_FROM_FILE
    ( "connect_in_read_same.xpnet"
    , connect
    , "connect-read P <-> A (existing connection is connect-in)"
    , 10, 7
    , 12, 7
    );
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_connect_in_in, fixture)
{
  GENERIC_DUPLICATE_FROM_FILE
    ( "connect_in_in_same.xpnet"
    , connect
    , "connect-in P <-> A (existing connection is connect-in)"
    , 11, 7
    , 12, 7
    );
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_connect_read_read, fixture)
{
  GENERIC_DUPLICATE_FROM_FILE
    ( "connect_read_read_same.xpnet"
    , connect
    , "connect-read P <-> A (existing connection is connect-read)"
    , 11, 7
    , 12, 7
    );
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_connect_out_out, fixture)
{
  GENERIC_DUPLICATE_FROM_FILE
    ( "connect_out_out_same.xpnet"
    , connect
    , "connect-out P <-> A (existing connection is connect-out)"
    , 11, 7
    , 12, 7
    );
}

#define GENERIC_DUPLICATE(_type,_msg,_le,_ce,_ll,_cl)                   \
  GENERIC_DUPLICATE_FROM_FILE                                           \
    ( "diagnostics/error_duplicate_" #_type ".xpnet"                    \
    , _type,_msg,_le,_ce,_ll,_cl                                        \
    )

BOOST_FIXTURE_TEST_CASE (error_duplicate_specialize, fixture)
{
  GENERIC_DUPLICATE (specialize,"specialize s",6,5,7,5);
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_place, fixture)
{
  GENERIC_DUPLICATE (place,"place p",3,5,4,5);
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_transition, fixture)
{
  GENERIC_DUPLICATE (transition,"transition t",3,5,4,5);
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_template, fixture)
{
  GENERIC_DUPLICATE (template,"template t",3,5,6,5);
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_port, fixture)
{
  GENERIC_DUPLICATE (port,"in-port p",2,3,3,3);
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_place_map, fixture)
{
  GENERIC_DUPLICATE (place_map,"place-map v <-> r",10,7,11,7);
}

#undef GENERIC_DUPLICATE

BOOST_FIXTURE_TEST_CASE (err1_invalid_closing_tag_name, fixture)
{
  std::string const input
    ( fmt::format (R"EOS(
<defun name="{0}">
  <expression>
</defun>)EOS"
      , fhg::util::testing::random_identifier()
      )
    );

  fhg::util::testing::require_exception_with_message
    <std::runtime_error>
    ( [&input]()
      { xml::parse::state::type state_empty;
        std::istringstream input_stream (input);
        xml::parse::just_parse (state_empty, input_stream);
      }
    , fmt::format ( "Parse error: {0}: invalid closing tag name"
                  , "[<stdin>:4:7]"
                  )
    );
}
