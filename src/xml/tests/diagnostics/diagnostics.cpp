// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE parser_diagnostics

#include <xml/parse/error.hpp>
#include <xml/parse/warning.hpp>

#include <parser_fixture.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/random_string.hpp>
#include <util-generic/temporary_path.hpp>

namespace
{
  struct fixture : parser_fixture
  {
    template<typename Ex>
    void require_exception_from_generate_cpp
      ( const std::string& exception_name
      , const boost::format& expected_what
      )
    {
      const fhg::util::temporary_path tp;
      state.path_to_cpp() = boost::filesystem::path (tp).string();

      try
      {
        ::xml::parse::generate_cpp (*function, state);

        BOOST_FAIL (boost::format ("exception <%1%> missing") % exception_name);
      }
      catch (const Ex& e)
      {
        BOOST_REQUIRE_EQUAL (e.what(), expected_what.str().c_str());
      }
    }

    template<typename Ex>
    void require_exception_from_parse
      ( const std::string& exception_name
      , const boost::format& expected_what
      )
    {
      try
      {
        parse();

        BOOST_FAIL (boost::format ("exception <%1%> missing") % exception_name);
      }
      catch (const Ex& e)
      {
        BOOST_REQUIRE_EQUAL (e.what(), expected_what.str().c_str());
      }
    }
  };
}

BOOST_AUTO_TEST_CASE (warning_struct_redefined)
{
  std::string const name_struct (fhg::util::testing::random_identifier());

  std::string const input
    ( ( boost::format (R"EOS(
<defun name="%1%">
  <struct name="%2%"><field name="%3%" type="%4%"/></struct>
  <struct name="%2%"><field name="%3%" type="%4%"/></struct>
</defun>)EOS")
      % fhg::util::testing::random_identifier()
      % name_struct
      % fhg::util::testing::random_identifier()
      % fhg::util::testing::random_identifier()
      ).str()
    );

  fhg::util::testing::require_exception
    <xml::parse::warning::struct_redefined>
    ( [&input]()
      { xml::parse::state::type state;
        state.warning_struct_redefined() = true;
        state.warning_error() = true;
        std::istringstream input_stream (input);
        xml::parse::post_processing_passes
          (xml::parse::just_parse (state, input_stream), &state);
      }
    , boost::format ("WARNING: struct %1% at %2% redefined at %3%")
    % name_struct
    % "[<stdin>:3:3]"
    % "[<stdin>:4:3]"
    );
}

BOOST_FIXTURE_TEST_CASE (warning_duplicate_external_function, fixture)
{
  parse ("diagnostics/warning_duplicate_external_function.xpnet");

  state.warning_duplicate_external_function() = true;
  state.warning_error() = true;

  require_exception_from_generate_cpp
    <xml::parse::warning::duplicate_external_function>
    ( "warn::duplicate_external_function"
    , boost::format ( "WARNING: the external function f in module m"
                      " has multiple occurences in %1% and %2%"
                    )
    % xml::parse::util::position_type (nullptr, nullptr, xpnet, 10, 9)
    % xml::parse::util::position_type (nullptr, nullptr, xpnet, 5, 9)
    );
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_external_function, fixture)
{
  parse ("diagnostics/error_duplicate_external_function.xpnet");

  require_exception_from_generate_cpp
    <xml::parse::error::duplicate_external_function>
    ( "error::duplicate_external_function"
    , boost::format ( "ERROR: duplicate external function f in module m"
                      " has conflicting definition at %1%"
                      ", earlier definition is at %2%"
                    )
    % xml::parse::util::position_type (nullptr, nullptr, xpnet, 5, 9)
    % xml::parse::util::position_type (nullptr, nullptr, xpnet, 10, 9)
    );
}

#define GENERIC_DUPLICATE_FROM_FILE(_file,_type,_msg,_le,_ce,_ll,_cl)   \
  set_parse_input (_file);                                              \
                                                                        \
  require_exception_from_parse<xml::parse::error::duplicate_ ## _type>  \
  ( "error::duplicate_" #_type                                          \
  , boost::format ( "ERROR: duplicate " _msg " at %1%"                  \
                    ", earlier definition is at %2%"                    \
                  )                                                     \
  % xml::parse::util::position_type (nullptr, nullptr, xpnet, _ll, _cl)       \
  % xml::parse::util::position_type (nullptr, nullptr, xpnet, _le, _ce)       \
  )

BOOST_FIXTURE_TEST_CASE (error_duplicate_connect_in_read, fixture)
{
  GENERIC_DUPLICATE_FROM_FILE
    ( "connect_in_read_same.xpnet"
    , connect
    , "connect-read P <-> A (existing connection is connect-in)"
    , 12, 7
    , 14, 7
    );
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_connect_in_in, fixture)
{
  GENERIC_DUPLICATE_FROM_FILE
    ( "connect_in_in_same.xpnet"
    , connect
    , "connect-in P <-> A (existing connection is connect-in)"
    , 13, 7
    , 14, 7
    );
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_connect_read_read, fixture)
{
  GENERIC_DUPLICATE_FROM_FILE
    ( "connect_read_read_same.xpnet"
    , connect
    , "connect-read P <-> A (existing connection is connect-read)"
    , 13, 7
    , 14, 7
    );
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_connect_out_out, fixture)
{
  GENERIC_DUPLICATE_FROM_FILE
    ( "connect_out_out_same.xpnet"
    , connect
    , "connect-out P <-> A (existing connection is connect-out)"
    , 13, 7
    , 14, 7
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
