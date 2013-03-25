// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE parser_diagnostics

#include <xml/parse/error.hpp>
#include <xml/parse/warning.hpp>

#include <parser_fixture.hpp>

#include <fhg/util/temporary_path.hpp>

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
      state.path_to_cpp() = tp;

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

BOOST_FIXTURE_TEST_CASE (warning_duplicate_external_function, fixture)
{
  parse ("diagnostics/warning_duplicate_external_function.xpnet");

  state.Wduplicate_external_function() = true;
  state.Werror() = true;

  require_exception_from_generate_cpp
    <xml::parse::warning::duplicate_external_function>
    ( "warn::duplicate_external_function"
    , boost::format ( "WARNING: the external function f in module m"
                      " has multiple occurences in %1% and %2%"
                    )
    % xml::parse::util::position_type (NULL, NULL, xpnet, 10, 9)
    % xml::parse::util::position_type (NULL, NULL, xpnet, 5, 9)
    );
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_external_function, fixture)
{
  parse ("diagnostics/error_duplicate_external_function.xpnet");

  require_exception_from_generate_cpp
    <xml::parse::error::duplicate_external_function>
    ( "error::duplicate_external_function"
    , boost::format ( "ERROR: the external function f in module m"
                      " has different definitions in %1% and %2%"
                    )
    % xml::parse::util::position_type (NULL, NULL, xpnet, 10, 9)
    % xml::parse::util::position_type (NULL, NULL, xpnet, 5, 9)
    );
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_connect_in_read, fixture)
{
  set_parse_input ("connect_in_read_same.xpnet");

  require_exception_from_parse<xml::parse::error::duplicate_connect>
    ( "error::duplicate_connect"
    , boost::format ( "ERROR: duplicate connect-read P <-> A for transition"
                      " foo in %1% (existing connection is connect-in)"
                      " in %2% and %3%"
                    )
    % xml::parse::util::position_type (NULL, NULL, xpnet, 6, 5)
    % xml::parse::util::position_type (NULL, NULL, xpnet, 14, 7)
    % xml::parse::util::position_type (NULL, NULL, xpnet, 12, 7)
    );
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_connect_in_in, fixture)
{
  set_parse_input ("connect_in_in_same.xpnet");

  require_exception_from_parse<xml::parse::error::duplicate_connect>
    ( "error::duplicate_connect"
    , boost::format ( "ERROR: duplicate connect-in P <-> A for transition"
                      " foo in %1% (existing connection is connect-in)"
                      " in %2% and %3%"
                    )
    % xml::parse::util::position_type (NULL, NULL, xpnet, 6, 5)
    % xml::parse::util::position_type (NULL, NULL, xpnet, 14, 7)
    % xml::parse::util::position_type (NULL, NULL, xpnet, 13, 7)
    );
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_connect_read_read, fixture)
{
  set_parse_input ("connect_read_read_same.xpnet");

  require_exception_from_parse<xml::parse::error::duplicate_connect>
    ( "error::duplicate_connect"
    , boost::format ( "ERROR: duplicate connect-read P <-> A for transition"
                      " foo in %1% (existing connection is connect-read)"
                      " in %2% and %3%"
                    )
    % xml::parse::util::position_type (NULL, NULL, xpnet, 6, 5)
    % xml::parse::util::position_type (NULL, NULL, xpnet, 14, 7)
    % xml::parse::util::position_type (NULL, NULL, xpnet, 13, 7)
    );
}

BOOST_FIXTURE_TEST_CASE (error_duplicate_connect_out_out, fixture)
{
  set_parse_input ("connect_out_out_same.xpnet");

  require_exception_from_parse<xml::parse::error::duplicate_connect>
    ( "error::duplicate_connect"
    , boost::format ( "ERROR: duplicate connect-out P <-> A for transition"
                      " foo in %1% (existing connection is connect-out)"
                      " in %2% and %3%"
                    )
    % xml::parse::util::position_type (NULL, NULL, xpnet, 6, 5)
    % xml::parse::util::position_type (NULL, NULL, xpnet, 14, 7)
    % xml::parse::util::position_type (NULL, NULL, xpnet, 13, 7)
    );
}
