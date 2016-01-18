// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE parse_memory_transfer
#include <boost/test/unit_test.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/type/function.hpp>

//! \todo remove, at the moment needed to make net_type a complete type
#include <we/type/net.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random_string.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/format.hpp>

#include <string>

BOOST_AUTO_TEST_CASE (memory_get_is_stored_in_function)
{
  std::string const global (fhg::util::testing::random_content_string());
  std::string const local (fhg::util::testing::random_content_string());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-get><global>%1%</global><local>%2%</local></memory-get>
</defun>)EOS")
      % global
      % local
      ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::type::function_type const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE_EQUAL (function.memory_gets().size(), 1);
    BOOST_REQUIRE_EQUAL (function.memory_gets().begin()->global(), global);
    BOOST_REQUIRE_EQUAL (function.memory_gets().begin()->local(), local);
}

namespace
{
  std::string const attr_not_modified_in_module_call
    (boost::optional<bool> not_modified_in_module_call)
  {
    return not_modified_in_module_call
      ? ( boost::format (" not-modified-in-module-call=\"%1%\"")
        % (*not_modified_in_module_call ? "true" : "false")
        ).str()
      : ""
      ;
  }

  void check_memory_put_is_stored_in_function
    (boost::optional<bool> not_modified_in_module_call)
  {
    std::string const global (fhg::util::testing::random_content_string());
    std::string const local (fhg::util::testing::random_content_string());

    std::string const input
      ( ( boost::format (R"EOS(
<defun>
  <memory-put%3%><global>%1%</global><local>%2%</local></memory-put>
</defun>)EOS")
        % global
        % local
        % attr_not_modified_in_module_call (not_modified_in_module_call)
        ).str()
      );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::type::function_type const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE_EQUAL (function.memory_puts().size(), 1);
    BOOST_REQUIRE_EQUAL (function.memory_puts().begin()->global(), global);
    BOOST_REQUIRE_EQUAL (function.memory_puts().begin()->local(), local);
    BOOST_REQUIRE_EQUAL
      ( function.memory_puts().begin()->not_modified_in_module_call()
      , not_modified_in_module_call
      );
  }
}

BOOST_AUTO_TEST_CASE (memory_put_is_stored_in_function)
{
  check_memory_put_is_stored_in_function (boost::none);
  check_memory_put_is_stored_in_function (true);
  check_memory_put_is_stored_in_function (false);
}

namespace
{
  void check_memory_getput_is_stored_in_function
    (boost::optional<bool> not_modified_in_module_call)
  {
    std::string const global (fhg::util::testing::random_content_string());
    std::string const local (fhg::util::testing::random_content_string());

    std::string const input
      ( ( boost::format (R"EOS(
<defun>
  <memory-getput%3%><global>%1%</global><local>%2%</local></memory-getput>
</defun>)EOS")
        % global
        % local
        % attr_not_modified_in_module_call (not_modified_in_module_call)
        ).str()
      );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::type::function_type const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE_EQUAL (function.memory_getputs().size(), 1);
    BOOST_REQUIRE_EQUAL (function.memory_getputs().begin()->global(), global);
    BOOST_REQUIRE_EQUAL (function.memory_getputs().begin()->local(), local);
    BOOST_REQUIRE_EQUAL
      ( function.memory_getputs().begin()->not_modified_in_module_call()
      , not_modified_in_module_call
      );
  }
}

BOOST_AUTO_TEST_CASE (memory_getput_is_stored_in_function)
{
  check_memory_getput_is_stored_in_function (boost::none);
  check_memory_getput_is_stored_in_function (true);
  check_memory_getput_is_stored_in_function (false);
}

namespace
{
  void check_memory_transfers_are_stored_in_function
    ( std::size_t const num_get
    , std::size_t const num_put
    , std::size_t const num_getput
    )
  {
    std::string input;

    input += "<defun>";

    for (std::size_t _ (0); _ < num_get; ++_)
    {
      input += "<memory-get><global/><local/></memory-get>";
    }
    for (std::size_t _ (0); _ < num_put; ++_)
    {
      input += "<memory-put><global/><local/></memory-put>";
    }
    for (std::size_t _ (0); _ < num_getput; ++_)
    {
      input += "<memory-getput><global/><local/></memory-getput>";
    }

    input += "</defun>";

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::type::function_type const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE_EQUAL (function.memory_gets().size(), num_get);
    BOOST_REQUIRE_EQUAL (function.memory_puts().size(), num_put);
    BOOST_REQUIRE_EQUAL (function.memory_getputs().size(), num_getput);
  }
}

BOOST_AUTO_TEST_CASE (memory_transfers_are_stored_in_function)
{
  for (std::size_t num_get (0); num_get < 5; ++num_get)
  {
    for (std::size_t num_put (0); num_put < 5; ++num_put)
    {
      for (std::size_t num_getput (0); num_getput < 5; ++num_getput)
      {
        check_memory_transfers_are_stored_in_function
          (num_get, num_put, num_getput);
      }
    }
  }
}

namespace
{
  void test_memory_transfer_for_non_module_call_throws
    (std::string const& tag)
  {
    std::string const name_function (fhg::util::testing::random_identifier());

    std::string const input
      ( ( boost::format (R"EOS(
<defun name="%1%">
  <memory-get><global/><local/></memory-get>
  <%2%/>
</defun>)EOS")
        % name_function
        % tag
        ).str()
      );

    xml::parse::state::type state;

    fhg::util::testing::require_exception_with_message
      <xml::parse::error::memory_transfer_for_non_module>
      ( [&state, &input]()
      { std::istringstream input_stream (input);
        xml::parse::just_parse (state, input_stream);
      }
      , boost::format ("ERROR: non module call function 'Just %1%'"
                      " with %2% memory transfer%3%"
                      ", function defined at %4%"
                      ", memory transfer%3% defined at: get: %5%"
                      )
      %  name_function
      % "1"
      % ""
      % "[<stdin>:2:1]"
      % "([<stdin>:3:3])"
      );
  }

  void test_memory_transfers_for_non_module_call_throws
    (std::string const& tag)
  {
    std::string const name_function (fhg::util::testing::random_identifier());

    std::string const input
      ( ( boost::format (R"EOS(
<defun name="%1%">
  <memory-put><global/><local/></memory-put>
  <memory-getput><global/><local/></memory-getput>
  <memory-put><global/><local/></memory-put>
  <memory-getput><global/><local/></memory-getput>
  <%2%/>
</defun>)EOS")
        % name_function
        % tag
        ).str()
      );

    xml::parse::state::type state;

    fhg::util::testing::require_exception_with_message
      <xml::parse::error::memory_transfer_for_non_module>
      ( [&state, &input]()
      { std::istringstream input_stream (input);
        xml::parse::just_parse (state, input_stream);
      }
      , boost::format ("ERROR: non module call function 'Just %1%'"
                      " with %2% memory transfer%3%"
                      ", function defined at %4%"
                      ", memory transfer%3% defined at: put: %5%, getput: %6%"
                      )
      %  name_function
      % "4"
      % "s"
      % "[<stdin>:2:1]"
      % "([<stdin>:3:3], [<stdin>:5:3])"
      % "([<stdin>:4:3], [<stdin>:6:3])"
      );
  }
}

BOOST_AUTO_TEST_CASE (memory_transfer_for_expression_throws)
{
  test_memory_transfer_for_non_module_call_throws ("expression");
  test_memory_transfers_for_non_module_call_throws ("expression");
}

BOOST_AUTO_TEST_CASE (memory_transfer_for_net_throws)
{
  test_memory_transfer_for_non_module_call_throws ("net");
  test_memory_transfers_for_non_module_call_throws ("net");
}
