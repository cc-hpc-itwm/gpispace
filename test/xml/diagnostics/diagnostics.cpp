// Copyright (C) 2013-2016,2018,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/xml/parse/error.hpp>
#include <gspc/xml/parse/warning.hpp>

#include <parser_fixture.hpp>

#include <gspc/util/executable_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/random/string.hpp>
#include <gspc/testing/require_exception.hpp>

#include <gspc/xml/parse/util/position.formatter.hpp>
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
      std::filesystem::path const path
        (gspc::util::executable_path().parent_path() / "gen");
      state.path_to_cpp() = path.string();

      gspc::testing::require_exception_with_message<Ex>
        ( [this]
          {
            ::gspc::xml::parse::generate_cpp (*function, state);
          }
        , expected_what
        );
    }

    template<typename Ex>
    void require_exception_from_parse
      (std::string const& expected_what)
    {
      gspc::testing::require_exception_with_message<Ex>
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
  std::string const name_struct (gspc::testing::random_identifier());

  std::string const input
    ( fmt::format (R"EOS(
<defun name="{0}">
  <struct name="{1}"><field name="{2}" type="{3}"/></struct>
  <struct name="{1}"><field name="{2}" type="{3}"/></struct>
  <expression/>
</defun>)EOS"
      , gspc::testing::random_identifier()
      , name_struct
      , gspc::testing::random_identifier()
      , gspc::testing::random_identifier()
      )
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::warning::struct_redefined>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        state.warning_struct_redefined() = true;
        state.warning_error() = true;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
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
  std::string const name_struct (gspc::testing::random_identifier());

  std::string const input
    ( fmt::format (R"EOS(
<defun name="{0}">
  <struct name="{1}"><field name="a" type="{2}"/></struct>
  <struct name="{1}"><field name="b" type="{2}"/></struct>
  <expression/>
</defun>)EOS"
      , gspc::testing::random_identifier()
      , name_struct
      , gspc::testing::random_identifier()
      )
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::struct_redefined>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
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
  std::string const name_field {gspc::testing::random_identifier()};

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
      , gspc::testing::random_identifier()
      , gspc::testing::random_identifier()
      , name_field
      , gspc::testing::random_identifier()
      , gspc::testing::random_identifier()
      )
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::struct_field_redefined>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        gspc::xml::parse::just_parse (state, input_stream);
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
    <gspc::xml::parse::warning::duplicate_external_function>
    ( fmt::format ( "WARNING: the external function f in module m"
                    " has multiple occurences in {0} and {1}"
                  , gspc::xml::parse::util::position_type (nullptr, nullptr, xpnet, 10, 9)
                  , gspc::xml::parse::util::position_type (nullptr, nullptr, xpnet, 5, 9)
                  )
    );
}

BOOST_AUTO_TEST_CASE (error_connect_response_with_unknown_port)
{
  std::string const port (gspc::testing::random_identifier());

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
      , gspc::testing::random_identifier()
      , gspc::testing::random_identifier_without_leading_underscore()
      , port
      , gspc::testing::random_identifier()
      )
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::unknown_port_in_connect_response>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
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
  std::string const to (gspc::testing::random_identifier());

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
      , gspc::testing::random_identifier()
      , gspc::testing::random_identifier_without_leading_underscore()
      , gspc::testing::random_identifier_without_leading_underscore()
      , to
      )
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::unknown_to_in_connect_response>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
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
    (gspc::testing::random_identifier_without_leading_underscore());
  std::string const name (gspc::testing::random_identifier());

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
      , gspc::testing::random_identifier()
      , name
      , gspc::testing::random_identifier_without_leading_underscore()
      , to
      , gspc::testing::random_identifier_without_leading_underscore()
      )
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::invalid_signature_in_connect_response>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
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
    <gspc::xml::parse::error::duplicate_external_function>
    ( fmt::format ( "ERROR: duplicate external function f in module m"
                    " has conflicting definition at {0}"
                    ", earlier definition is at {1}"
                  , gspc::xml::parse::util::position_type (nullptr, nullptr, xpnet, 5, 9)
                  , gspc::xml::parse::util::position_type (nullptr, nullptr, xpnet, 10, 9)
                  )
    );
}

#define GENERIC_DUPLICATE_FROM_FILE(_file,_type,_msg,_le,_ce,_ll,_cl)   \
  set_parse_input (_file);                                              \
                                                                        \
  require_exception_from_parse<gspc::xml::parse::error::duplicate_ ## _type>  \
  ( fmt::format ( "ERROR: duplicate " _msg " at {0}"                \
                    ", earlier definition is at {1}"                    \
                  , gspc::xml::parse::util::position_type (nullptr, nullptr, xpnet, _ll, _cl)       \
                  , gspc::xml::parse::util::position_type (nullptr, nullptr, xpnet, _le, _ce)       \
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
  GENERIC_DUPLICATE (template,"template Just t",3,5,6,5);
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
      , gspc::testing::random_identifier()
      )
    );

  gspc::testing::require_exception_with_message
    <std::runtime_error>
    ( [&input]()
      { gspc::xml::parse::state::type state_empty;
        std::istringstream input_stream (input);
        gspc::xml::parse::just_parse (state_empty, input_stream);
      }
    , fmt::format ( "Parse error: {0}: invalid closing tag name"
                  , "[<stdin>:4:7]"
                  )
    );
}
BOOST_AUTO_TEST_CASE (error_shared_place_does_not_exist)
{
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="data" type="shared_cleanup"/>
    <transition name="t">
      <defun>
        <in name="x" type="shared_cleanup"/>
        <expression/>
      </defun>
      <connect-in port="x" place="data"/>
    </transition>
  </net>
</defun>)EOS"
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::shared_place_does_not_exist>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
        ( "ERROR: shared cleanup place 'cleanup' does not exist,"
          " referenced by type 'shared_cleanup' for place 'data' at {}"
        , "[<stdin>:4:5]"
        )
    );
}

BOOST_AUTO_TEST_CASE (error_shared_place_not_marked_as_sink)
{
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="cleanup" type="int"/>
    <place name="data" type="shared_cleanup"/>
    <transition name="t">
      <defun>
        <in name="x" type="shared_cleanup"/>
        <expression/>
      </defun>
      <connect-in port="x" place="data"/>
    </transition>
  </net>
</defun>)EOS"
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::shared_place_not_marked_as_sink>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
        ( "ERROR: shared cleanup place 'cleanup' at {} must have attribute"
          " shared_sink=\"true\", referenced by type 'shared_cleanup'"
          " for place 'data' at {}"
        , "[<stdin>:4:5]"
        , "[<stdin>:5:5]"
        )
    );
}

// Test that shared_sink places do NOT trigger the "independent place" warning
// even when they have no explicit connections (since the runtime writes to them)
BOOST_AUTO_TEST_CASE (shared_sink_place_does_not_trigger_independent_place_warning)
{
  // This input has a shared_sink place with no connections, which would normally
  // trigger the independent_place warning. But shared_sink should suppress it.
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="cleanup" type="int" shared_sink="true"/>
    <place name="data" type="shared_cleanup"/>
    <transition name="t">
      <defun>
        <in name="x" type="shared_cleanup"/>
        <expression/>
      </defun>
      <connect-in port="x" place="data"/>
    </transition>
  </net>
</defun>)EOS"
    );

  gspc::xml::parse::state::type state;
  state.warning_error() = true;  // Make warnings into errors
  state.warning_independent_place() = true;  // Enable independent place warning

  std::istringstream input_stream (input);

  // Should NOT throw - the shared_sink place should not trigger the warning
  // The warning check is performed during synthesis (xml_to_we)
  BOOST_CHECK_NO_THROW
    ( [&]
      {
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
        gspc::xml::parse::xml_to_we (function, state);  // This performs the independent_place check
      }()
    );
}

// Test that a non-shared_sink place DOES trigger the independent place warning
BOOST_AUTO_TEST_CASE (non_shared_sink_unconnected_place_triggers_warning)
{
  // This input has an unconnected place without shared_sink, which should
  // trigger the independent_place warning during synthesis (xml_to_we)
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="unconnected" type="int"/>
    <place name="data" type="int">
      <token><value>42</value></token>
    </place>
    <transition name="t">
      <defun>
        <in name="x" type="int"/>
        <expression/>
      </defun>
      <connect-in port="x" place="data"/>
    </transition>
  </net>
</defun>)EOS"
    );

  gspc::xml::parse::state::type state;
  state.warning_error() = true;  // Make warnings into errors
  state.warning_independent_place() = true;  // Enable independent place warning

  std::istringstream input_stream (input);

  // Should throw the independent_place warning (as error)
  // The warning is triggered during synthesis (xml_to_we), not during parsing
  // It is wrapped in a nested exception by synthesize(), so we use make_nested
  gspc::testing::require_exception
    ( [&]
      {
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
        gspc::xml::parse::xml_to_we (function, state);  // This triggers independent_place check
      }
    , gspc::testing::make_nested
        ( std::runtime_error ("In 'test' defined at [<stdin>:2:1].")
        , gspc::xml::parse::warning::independent_place ("unconnected", "<stdin>")
        )
    );
}

BOOST_AUTO_TEST_CASE (error_contradicting_shared_attributes_passthrough_and_produce)
{
  auto const name_transition
    {gspc::testing::random_identifier_without_leading_underscore()};

  auto const input
    { fmt::format (R"EOS(
<defun name="test">
  <net>
    <transition name="{0}" passthrough_shared="true" produce_shared="true">
      <defun>
        <expression/>
      </defun>
    </transition>
  </net>
</defun>)EOS"
      , name_transition
      )
    };

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::contradicting_shared_attributes>
    ( [&input]
      {
        gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        gspc::xml::parse::just_parse (state, input_stream);
      }
    , fmt::format ( "ERROR: contradicting shared tracking attributes:"
                    " passthrough_shared cannot be combined with"
                    " produce_shared or consume_shared"
                    " in transition {} at [<stdin>:4:5]"
                  , name_transition
                  )
    );
}

BOOST_AUTO_TEST_CASE (error_contradicting_shared_attributes_passthrough_and_consume)
{
  auto const name_transition
    {gspc::testing::random_identifier_without_leading_underscore()};

  auto const input
    { fmt::format (R"EOS(
<defun name="test">
  <net>
    <transition name="{0}" passthrough_shared="true" consume_shared="true">
      <defun>
        <expression/>
      </defun>
    </transition>
  </net>
</defun>)EOS"
      , name_transition
      )
    };

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::contradicting_shared_attributes>
    ( [&input]
      {
        gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        gspc::xml::parse::just_parse (state, input_stream);
      }
    , fmt::format ( "ERROR: contradicting shared tracking attributes:"
                    " passthrough_shared cannot be combined with"
                    " produce_shared or consume_shared"
                    " in transition {} at [<stdin>:4:5]"
                  , name_transition
                  )
    );
}

BOOST_AUTO_TEST_CASE (error_contradicting_shared_attributes_passthrough_and_both)
{
  auto const name_transition
    {gspc::testing::random_identifier_without_leading_underscore()};

  auto const input
    { fmt::format (R"EOS(
<defun name="test">
  <net>
    <transition name="{0}" passthrough_shared="true" produce_shared="true" consume_shared="true">
      <defun>
        <expression/>
      </defun>
    </transition>
  </net>
</defun>)EOS"
      , name_transition
      )
    };

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::contradicting_shared_attributes>
    ( [&input]
      {
        gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        gspc::xml::parse::just_parse (state, input_stream);
      }
    , fmt::format ( "ERROR: contradicting shared tracking attributes:"
                    " passthrough_shared cannot be combined with"
                    " produce_shared or consume_shared"
                    " in transition {} at [<stdin>:4:5]"
                  , name_transition
                  )
    );
}

BOOST_AUTO_TEST_CASE (no_error_for_redundant_shared_attributes)
{
  // passthrough_shared=true with produce_shared=false and consume_shared=false
  // is redundant but not contradictory, so it should be accepted
  auto const input
    { R"EOS(
<defun name="test">
  <net>
    <transition name="redundant" passthrough_shared="true" produce_shared="false" consume_shared="false">
      <defun>
        <expression/>
      </defun>
    </transition>
  </net>
</defun>)EOS"
    };

  gspc::xml::parse::state::type state;
  std::istringstream input_stream (input);
  BOOST_REQUIRE_NO_THROW (gspc::xml::parse::just_parse (state, input_stream));
}

// --- Generator place validation tests ---

BOOST_AUTO_TEST_CASE (error_generator_place_with_tokens)
{
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="gen" type="int" generator="true">
      <token><value>42</value></token>
    </place>
  </net>
</defun>)EOS"
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::generator_place_with_tokens>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
        ( "ERROR: generator place 'gen' at {} must not have <token> elements"
        , "[<stdin>:4:5]"
        )
    );
}

BOOST_AUTO_TEST_CASE (error_generator_place_with_put_token)
{
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="gen" type="int" generator="true" put_token="true"/>
    <transition name="t">
      <defun>
        <out name="x" type="int"/>
        <expression>x := 1</expression>
      </defun>
      <connect-out port="x" place="gen"/>
    </transition>
  </net>
</defun>)EOS"
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::generator_place_with_put_token>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
        ( "ERROR: generator place 'gen' at {} must not have"
          " attribute put_token=\"true\""
        , "[<stdin>:4:5]"
        )
    );
}

BOOST_AUTO_TEST_CASE (error_generator_place_with_shared_sink)
{
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="gen" type="int" generator="true" shared_sink="true"/>
  </net>
</defun>)EOS"
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::generator_place_with_shared_sink>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
        ( "ERROR: generator place 'gen' at {} must not have"
          " attribute shared_sink=\"true\""
        , "[<stdin>:4:5]"
        )
    );
}

BOOST_AUTO_TEST_CASE (error_generator_place_invalid_type)
{
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="gen" type="float" generator="true"/>
  </net>
</defun>)EOS"
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::generator_place_invalid_type>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
        ( "ERROR: generator place 'gen' at {} has invalid type 'float',"
          " must be one of: int, long, unsigned int, unsigned long,"
          " char, string, bigint"
        , "[<stdin>:4:5]"
        )
    );
}

BOOST_AUTO_TEST_CASE (error_generator_place_with_read_connection)
{
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="gen" type="int" generator="true"/>
    <transition name="t">
      <defun>
        <in name="x" type="int"/>
        <expression/>
      </defun>
      <connect-read port="x" place="gen"/>
    </transition>
  </net>
</defun>)EOS"
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::generator_place_with_read_connection>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
        ( "ERROR: generator place 'gen' at {} must not have read connections,"
          " but transition 't' at {} has a read connection to it"
        , "[<stdin>:4:5]"
        , "[<stdin>:5:5]"
        )
    );
}

BOOST_AUTO_TEST_CASE (error_generator_place_with_connect_out)
{
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="gen" type="int" generator="true"/>
    <transition name="t">
      <defun>
        <out name="x" type="int"/>
        <expression>x := 1</expression>
      </defun>
      <connect-out port="x" place="gen"/>
    </transition>
  </net>
</defun>)EOS"
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::generator_place_with_incoming_connection>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
        ( "ERROR: generator place 'gen' at {}"
          " must not have incoming connections,"
          " but transition 't' at {} has a connection to it"
        , "[<stdin>:4:5]"
        , "[<stdin>:5:5]"
        )
    );
}

BOOST_AUTO_TEST_CASE (error_generator_place_with_connect_inout)
{
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="gen" type="int" generator="true"/>
    <transition name="t">
      <defun>
        <inout name="x" type="int"/>
        <expression/>
      </defun>
      <connect-inout port="x" place="gen"/>
    </transition>
  </net>
</defun>)EOS"
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::generator_place_with_incoming_connection>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
        ( "ERROR: generator place 'gen' at {}"
          " must not have incoming connections,"
          " but transition 't' at {} has a connection to it"
        , "[<stdin>:4:5]"
        , "[<stdin>:5:5]"
        )
    );
}

BOOST_AUTO_TEST_CASE (error_generator_place_with_connect_out_many)
{
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="gen" type="string" generator="true"/>
    <place name="items" type="list"/>
    <transition name="t">
      <defun>
        <in name="items" type="list"/>
        <out name="x" type="string"/>
        <expression/>
      </defun>
      <connect-in port="items" place="items"/>
      <connect-out-many port="x" place="gen"/>
    </transition>
  </net>
</defun>)EOS"
    );

  gspc::testing::require_exception_with_message
    <gspc::xml::parse::error::generator_place_with_incoming_connection>
    ( [&input]()
      { gspc::xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }
    , fmt::format
        ( "ERROR: generator place 'gen' at {}"
          " must not have incoming connections,"
          " but transition 't' at {} has a connection to it"
        , "[<stdin>:4:5]"
        , "[<stdin>:6:5]"
        )
    );
}

BOOST_AUTO_TEST_CASE (valid_generator_place)
{
  // A valid generator place with an in connection should parse without errors
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="gen" type="int" generator="true"/>
    <place name="out" type="int"/>
    <transition name="t">
      <defun>
        <in name="x" type="int"/>
        <out name="y" type="int"/>
        <expression>y := x</expression>
      </defun>
      <connect-in port="x" place="gen"/>
      <connect-out port="y" place="out"/>
    </transition>
  </net>
</defun>)EOS"
    );

  gspc::xml::parse::state::type state;
  std::istringstream input_stream (input);
  BOOST_REQUIRE_NO_THROW
    ( [&]
      {
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }()
    );
}

BOOST_AUTO_TEST_CASE (valid_generator_place_with_supported_types)
{
  // Test all supported generator types
  std::vector<std::string> const supported_types
    {"int", "long", "unsigned int", "unsigned long", "char", "string", "bigint"};

  for (auto const& type : supported_types)
  {
    std::string const input
      { fmt::format (R"EOS(
<defun name="test">
  <net>
    <place name="gen" type="{}" generator="true"/>
    <place name="out" type="{}"/>
    <transition name="t">
      <defun>
        <in name="x" type="{}"/>
        <out name="y" type="{}"/>
        <expression>y := x</expression>
      </defun>
      <connect-in port="x" place="gen"/>
      <connect-out port="y" place="out"/>
    </transition>
  </net>
</defun>)EOS"
        , type, type, type, type
        )
      };

    gspc::xml::parse::state::type state;
    std::istringstream input_stream (input);
    BOOST_REQUIRE_NO_THROW
      ( [&]
        {
          auto function (gspc::xml::parse::just_parse (state, input_stream));
          gspc::xml::parse::post_processing_passes (function, &state);
        }()
      );
  }
}

// Test that generator="false" (explicit) is treated the same as no attribute
BOOST_AUTO_TEST_CASE (generator_false_is_not_generator)
{
  std::string const input
    (R"EOS(
<defun name="test">
  <net>
    <place name="p" type="float" generator="false"/>
  </net>
</defun>)EOS"
    );

  // float type is invalid for generators, but generator="false" means it's
  // not a generator, so this should succeed
  gspc::xml::parse::state::type state;
  std::istringstream input_stream (input);
  BOOST_REQUIRE_NO_THROW
    ( [&]
      {
        auto function (gspc::xml::parse::just_parse (state, input_stream));
        gspc::xml::parse::post_processing_passes (function, &state);
      }()
    );
}
