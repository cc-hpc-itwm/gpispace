// mirko.rahn@itwm.fraunhofer.de

#include <boost/test/unit_test.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/type/function.hpp>

//! \todo remove, at the moment needed to make net_type a complete type
#include <we/type/net.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <fhg/util/boost/variant.hpp>

#include <boost/format.hpp>

#include <string>

namespace
{
  std::string random_identifier_with_valid_prefix()
  {
    return
      fhg::util::testing::random_char_of
      ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
      +
      fhg::util::testing::random_string_of
      ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
  }
}

namespace
{
  void test_buffer_without_size(bool is_buf_cached)
  {
    std::string const name (random_identifier_with_valid_prefix());

    std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <%1%memory-buffer name="%2%"/>
  <expression/>
</defun>)EOS")
    % (is_buf_cached?"cached-":"")
    % name
    ).str()
    );

    xml::parse::state::type state;

    fhg::util::testing::require_exception_with_message
    <xml::parse::error::memory_buffer_without_size>
    ( [&state, &input]()
        { std::istringstream input_stream (input);
        xml::parse::just_parse (state, input_stream);
        }
    , boost::format ("ERROR: memory-buffer '%1%' without size, at %2%")
    % name
    % "[<stdin>:3:3]"
    );
  }
}

BOOST_AUTO_TEST_CASE (memory_buffer_without_size_throws)
{
  test_buffer_without_size(true);
  test_buffer_without_size(false);
}

BOOST_AUTO_TEST_CASE (cached_memory_buffer_without_dataid_throws)
{
  std::string const name_memory_buffer (random_identifier_with_valid_prefix());

  std::string const input
  ( ( boost::format (R"EOS(
<defun>
  <cached-memory-buffer name="%1%"><size/></cached-memory-buffer>
  <expression/>
</defun>)EOS")
  % name_memory_buffer
  ).str()
  );

  xml::parse::state::type state;

  fhg::util::testing::require_exception_with_message
  <xml::parse::error::cached_memory_buffer_without_dataid>
  ( [&state, &input]()
      { std::istringstream input_stream (input);
      xml::parse::just_parse (state, input_stream);
      }
  , boost::format ("ERROR: cached-memory-buffer '%1%' without dataid, at %2%")
  % name_memory_buffer
  % "[<stdin>:3:3]"
  );
}

BOOST_AUTO_TEST_CASE (cached_memory_buffer_without_memory_get_throws)
{
  std::string const name_memory_buffer (random_identifier_with_valid_prefix());

  std::string const input
  ( ( boost::format (R"EOS(
<defun>
  <cached-memory-buffer name="%1%"><size/><dataid/></cached-memory-buffer>
  <expression/>
</defun>)EOS")
  % name_memory_buffer
  ).str()
  );

  xml::parse::state::type state;

  fhg::util::testing::require_exception_with_message
  <xml::parse::error::cached_memory_buffer_without_mem_get>
  ( [&state, &input]()
      { std::istringstream input_stream (input);
      xml::parse::just_parse (state, input_stream);
      }
  , boost::format ("ERROR: cached-memory-buffer '%1%' without memory-get, at %2%")
  % name_memory_buffer
  % "[<stdin>:3:3]"
  );
}

namespace
{
  void test_duplicate_memory_buffer(bool is_buf1_cached, bool is_buf2_cached)
  {
    std::string const name (random_identifier_with_valid_prefix());

    std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <%2%memory-buffer name="%1%"><size/>%4%</%2%memory-buffer>
  <%3%memory-buffer name="%1%"><size/>%5%</%3%memory-buffer>
  <expression/>
</defun>)EOS")
    % name
    % (is_buf1_cached?"cached-":"")
    % (is_buf2_cached?"cached-":"")
    % (is_buf1_cached?"<dataid/><memory-get/>":"")
    % (is_buf2_cached?"<dataid/><memory-get/>":"")
    ).str()
    );

    xml::parse::state::type state;

    fhg::util::testing::require_exception_with_message
    <xml::parse::error::duplicate_memory_buffer>
    ( [&state, &input]()
        { std::istringstream input_stream (input);
        xml::parse::just_parse (state, input_stream);
        }
    , boost::format ("ERROR: duplicate memory-buffer '%1%'"
        " at %2%, earlier definition is at %3%"
    )
    % name
    % "[<stdin>:4:3]"
    % "[<stdin>:3:3]"
    );
  }
}

BOOST_AUTO_TEST_CASE (duplicate_memory_buffer_throws)
{
  test_duplicate_memory_buffer(false, false);
  test_duplicate_memory_buffer(false, true);
  test_duplicate_memory_buffer(true, false);
  test_duplicate_memory_buffer(true, true);
}

namespace
{
  void test_memory_buffer_is_stored_in_function(bool is_buf_cached)
  {
    std::string const name (random_identifier_with_valid_prefix());
    std::string const size (fhg::util::testing::random_content_string());

    std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <%5%memory-buffer name="%1%"><size>%2%</size>%6%</%5%memory-buffer>
  <module name="%3%" function="%4%"/>
</defun>)EOS")
    % name
    % size
    % fhg::util::testing::random_identifier()
    % fhg::util::testing::random_identifier()
    % (is_buf_cached?"cached-":"")
    % (is_buf_cached?"<dataid/><memory-get/>":"")
    ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::type::function_type const function
    (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE_EQUAL (function.memory_buffers().size(), 1);
    BOOST_REQUIRE_EQUAL ( function.memory_buffers().begin()->name()
        , name
    );
    BOOST_REQUIRE_EQUAL ( function.memory_buffers().begin()->size()
        , size
    );
  }
}

BOOST_AUTO_TEST_CASE (memory_buffer_is_stored_in_function)
{
  test_memory_buffer_is_stored_in_function(false);
  test_memory_buffer_is_stored_in_function(true);
}

namespace
{
  void test_memory_buffer_are_stored_in_function(bool is_buf1_cached, bool is_buf2_cached)
  {
    std::string const name_1 (random_identifier_with_valid_prefix());
    std::string const size_1 (fhg::util::testing::random_content_string());
    std::string const name_2 (random_identifier_with_valid_prefix());
    std::string const size_2 (fhg::util::testing::random_content_string());

    std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <%7%memory-buffer name="%1%"><size>%2%</size>%9%</%7%memory-buffer>
  <%8%memory-buffer name="%3%"><size>%4%</size>%10%</%8%memory-buffer>
  <module name="%5%" function="%6%"/>
</defun>)EOS")
    % name_1
    % size_1
    % name_2
    % size_2
    % fhg::util::testing::random_identifier()
    % fhg::util::testing::random_identifier()
    % (is_buf1_cached?"cached-":"")
    % (is_buf2_cached?"cached-":"")
    % (is_buf1_cached?"<dataid/><memory-get/>":"")
    % (is_buf2_cached?"<dataid/><memory-get/>":"")
    ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::type::function_type const function
    (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE_EQUAL (function.memory_buffers().size(), 2);
    BOOST_REQUIRE (function.memory_buffers().has (name_1));
    BOOST_REQUIRE (function.memory_buffers().has (name_2));
    BOOST_REQUIRE_EQUAL
    (function.memory_buffers().get (name_1)->size(), size_1);
    BOOST_REQUIRE_EQUAL
    (function.memory_buffers().get (name_2)->size(), size_2);
  }
}

BOOST_AUTO_TEST_CASE (memory_buffers_are_stored_in_function)
{
  test_memory_buffer_are_stored_in_function(false, false);
  test_memory_buffer_are_stored_in_function(false, true);
  test_memory_buffer_are_stored_in_function(true, false);
  test_memory_buffer_are_stored_in_function(true, true);
}

namespace
{
  void test_memory_buffer_for_non_module_call_throws
    (std::string const& tag, bool is_buf_cached)
  {
    std::string const name_function (fhg::util::testing::random_identifier());

    std::string const input
      ( ( boost::format (R"EOS(
<defun name="%1%">
  <%4%memory-buffer name="%3%"><size/>%5%</%4%memory-buffer>
  <%2%/>
</defun>)EOS")
        % name_function
        % tag
        % random_identifier_with_valid_prefix()
        % (is_buf_cached?"cached-":"")
        % (is_buf_cached?"<dataid/><memory-get/>":"")
        ).str()
      );

    xml::parse::state::type state;

    fhg::util::testing::require_exception_with_message
      <xml::parse::error::memory_buffer_for_non_module>
      ( [&state, &input]()
      { std::istringstream input_stream (input);
        xml::parse::just_parse (state, input_stream);
      }
      , boost::format ("ERROR: non module call function 'Just %1%'"
                      " with %2% memory buffer%3%"
                      ", function defined at %4%"
                      ", memory buffer%3% defined at %5%"
                      )
      %  name_function
      % "1"
      % ""
      % "[<stdin>:2:1]"
      % "{[<stdin>:3:3]}"
      );
  }

  void test_memory_buffers_for_non_module_call_throws (std::string const& tag,
      bool is_buf1_cached, bool is_buf2_cached)
  {
    std::string const name_function (fhg::util::testing::random_identifier());

    std::string const input
      ( ( boost::format (R"EOS(
<defun name="%1%">
  <%5%memory-buffer name="%3%"><size/>%7%</%5%memory-buffer>
  <%6%memory-buffer name="%4%"><size/>%8%</%6%memory-buffer>
  <%2%/>
</defun>)EOS")
        % name_function
        % tag
        % random_identifier_with_valid_prefix()
        % random_identifier_with_valid_prefix()
        % (is_buf1_cached?"cached-":"")
        % (is_buf2_cached?"cached-":"")
        % (is_buf1_cached?"<dataid/><memory-get/>":"")
        % (is_buf2_cached?"<dataid/><memory-get/>":"")
        ).str()
      );

    xml::parse::state::type state;

    fhg::util::testing::require_exception_with_message_in
      <xml::parse::error::memory_buffer_for_non_module>
      ( [&state, &input]()
      { std::istringstream input_stream (input);
        xml::parse::just_parse (state, input_stream);
      }
      , { boost::format ("ERROR: non module call function 'Just %1%'"
                        " with %2% memory buffer%3%"
                        ", function defined at %4%"
                        ", memory buffer%3% defined at %5%"
                        )
        %  name_function
        % "2"
        % "s"
        % "[<stdin>:2:1]"
        % "{[<stdin>:3:3], [<stdin>:4:3]}"
        , boost::format ("ERROR: non module call function 'Just %1%'"
                        " with %2% memory buffer%3%"
                        ", function defined at %4%"
                        ", memory buffer%3% defined at %5%"
                        )
        %  name_function
        % "2"
        % "s"
        % "[<stdin>:2:1]"
        % "{[<stdin>:4:3], [<stdin>:3:3]}"
        }
      );
  }
}

BOOST_AUTO_TEST_CASE (memory_buffer_for_expression_throws)
{
  test_memory_buffer_for_non_module_call_throws ("expression", false);
  test_memory_buffer_for_non_module_call_throws ("expression", true);

  test_memory_buffers_for_non_module_call_throws ("expression", false, false);
  test_memory_buffers_for_non_module_call_throws ("expression", false, true);
  test_memory_buffers_for_non_module_call_throws ("expression", true, false);
  test_memory_buffers_for_non_module_call_throws ("expression", true, true);
}

BOOST_AUTO_TEST_CASE (memory_buffer_for_net_throws)
{
  test_memory_buffer_for_non_module_call_throws ("net", false);
  test_memory_buffer_for_non_module_call_throws ("net", true);

  test_memory_buffers_for_non_module_call_throws ("net", false, false);
  test_memory_buffers_for_non_module_call_throws ("net", false, true);
  test_memory_buffers_for_non_module_call_throws ("net", true, false);
  test_memory_buffers_for_non_module_call_throws ("net", true, true);
}

namespace
{
  void test_memory_buffer_with_the_same_name_as_a_port_throws
    (std::string const& port_direction, bool is_buf_cached)
  {
    std::string const name_port_and_memory_buffer
      (random_identifier_with_valid_prefix());

    std::string const input
      ( ( boost::format (R"EOS(
<defun>
  <%2% name="%1%" type=""/>
  <%5%memory-buffer name="%1%"><size/>%6%</%5%memory-buffer>
  <module name="%3%" function="%4%"/>
</defun>)EOS")
        % name_port_and_memory_buffer
        % port_direction
        % fhg::util::testing::random_identifier()
        % fhg::util::testing::random_identifier()
        % (is_buf_cached?"cached-":"")
        % (is_buf_cached?"<dataid/><memory-get/>":"")
        ).str()
      );

    xml::parse::state::type state;

    fhg::util::testing::require_exception_with_message
      <xml::parse::error::memory_buffer_with_same_name_as_port>
      ( [&state, &input]()
      { std::istringstream input_stream (input);
        xml::parse::just_parse (state, input_stream);
      }
      , boost::format ("ERROR: memory buffer '%1%' defined at %2%"
                      " with the same name as the %3%-port defined at %4%"
                      )
      %  name_port_and_memory_buffer
      % "[<stdin>:4:3]"
      % port_direction
      % "[<stdin>:3:3]"
      );
  }
}

BOOST_AUTO_TEST_CASE (memory_buffer_with_the_same_name_as_a_port_throws)
{
  test_memory_buffer_with_the_same_name_as_a_port_throws ("in", false);
  test_memory_buffer_with_the_same_name_as_a_port_throws ("out", false);

  test_memory_buffer_with_the_same_name_as_a_port_throws ("in", true);
  test_memory_buffer_with_the_same_name_as_a_port_throws ("out", true);
}

namespace
{
  void test_memory_buffer_with_the_same_name_as_a_port_added_later_throws
    (std::string const& port_direction, bool is_buf_cached)
  {
    std::string const name_port_added_earlier
      (random_identifier_with_valid_prefix());
    std::string const name_port_and_memory_buffer
      (random_identifier_with_valid_prefix());

    std::string const input
      ( ( boost::format (R"EOS(
<defun>
  <in name="%1%" type=""/>
  <%6%memory-buffer name="%2%"><size/>%7%</%6%memory-buffer>
  <%3% name="%2%" type=""/>
  <module name="%4%" function="%5%"/>
</defun>)EOS")
        % name_port_added_earlier
        % name_port_and_memory_buffer
        % port_direction
        % fhg::util::testing::random_identifier()
        % fhg::util::testing::random_identifier()
        % (is_buf_cached?"cached-":"")
        % (is_buf_cached?"<dataid/><memory-get/>":"")
        ).str()
      );

    xml::parse::state::type state;

    fhg::util::testing::require_exception_with_message
      <xml::parse::error::memory_buffer_with_same_name_as_port>
      ( [&state, &input]()
      { std::istringstream input_stream (input);
        xml::parse::just_parse (state, input_stream);
      }
      , boost::format ("ERROR: memory buffer '%1%' defined at %2%"
                      " with the same name as the %3%-port defined at %4%"
                      )
      %  name_port_and_memory_buffer
      % "[<stdin>:4:3]"
      % port_direction
      % "[<stdin>:5:3]"
      );
  }
}

BOOST_AUTO_TEST_CASE
  (memory_buffer_with_the_same_name_as_a_port_added_later_throws)
{
  test_memory_buffer_with_the_same_name_as_a_port_added_later_throws ("in", false);
  test_memory_buffer_with_the_same_name_as_a_port_added_later_throws ("out", false);

  test_memory_buffer_with_the_same_name_as_a_port_added_later_throws ("in", true);
  test_memory_buffer_with_the_same_name_as_a_port_added_later_throws ("out", true);
}

namespace
{
  void test_memory_buffer_accepted_as_argument_in_function_signature(bool is_buf_cached)
  {
    std::string const name_memory_buffer (random_identifier_with_valid_prefix());

    std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <%3%memory-buffer name="%1%"><size/>%4%</%3%memory-buffer>
  <module name="" function="%2% (%1%)"/>
</defun>)EOS")
    % name_memory_buffer
    % random_identifier_with_valid_prefix()
    % (is_buf_cached?"cached-":"")
    % (is_buf_cached?"<dataid/><memory-get/>":"")
    ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::type::function_type const function
    (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE ( fhg::util::boost::is_of_type<xml::parse::type::module_type>
    (function.content())
    );

    xml::parse::type::module_type const& module_call
    ( boost::get<xml::parse::type::module_type>
    (function.content())
    );

    BOOST_REQUIRE_EQUAL (module_call.memory_buffer_arg().size(), 1);
    BOOST_REQUIRE_EQUAL ( *(module_call.memory_buffer_arg().begin())
        , name_memory_buffer
    );
  }
}

BOOST_AUTO_TEST_CASE (memory_buffer_accepted_as_argument_in_function_signature)
{
  test_memory_buffer_accepted_as_argument_in_function_signature(false);
  test_memory_buffer_accepted_as_argument_in_function_signature(true);
}

namespace
{
  void test_memory_buffers_accepted_as_arguments_in_function_signature
    (bool is_buf1_cached, bool is_buf2_cached)
  {
  std::string const name_memory_buffer_A
    (random_identifier_with_valid_prefix());
  std::string const name_memory_buffer_B
    (random_identifier_with_valid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <%4%memory-buffer name="%1%"><size/>%6%</%4%memory-buffer>
  <%5%memory-buffer name="%2%"><size/>%7%</%5%memory-buffer>
  <module name="" function="%3% (%1%, %2%)"/>
</defun>)EOS")
      % name_memory_buffer_A
      % name_memory_buffer_B
      % fhg::util::testing::random_identifier()
      % (is_buf1_cached?"cached-":"")
      % (is_buf2_cached?"cached-":"")
      % (is_buf1_cached?"<dataid/><memory-get/>":"")
      % (is_buf2_cached?"<dataid/><memory-get/>":"")
      ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::type::function_type const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE ( fhg::util::boost::is_of_type<xml::parse::type::module_type>
                    (function.content())
                  );

    xml::parse::type::module_type const& module_call
      ( boost::get<xml::parse::type::module_type>
        (function.content())
      );

    BOOST_REQUIRE_EQUAL (module_call.memory_buffer_arg().size(), 2);
    BOOST_REQUIRE_EQUAL ( *(module_call.memory_buffer_arg().begin())
                        , name_memory_buffer_A
                        );
    BOOST_REQUIRE_EQUAL
      ( *(std::next (module_call.memory_buffer_arg().begin()))
      , name_memory_buffer_B
      );
  }
}

BOOST_AUTO_TEST_CASE (memory_buffers_accepted_as_arguments_in_function_signature)
{
  test_memory_buffers_accepted_as_arguments_in_function_signature(false, false);
  test_memory_buffers_accepted_as_arguments_in_function_signature(false, true);
  test_memory_buffers_accepted_as_arguments_in_function_signature(true, false);
  test_memory_buffers_accepted_as_arguments_in_function_signature(true, true);
}


namespace
{
  void test_memory_buffer_mixed_with_port_accepted_as_argument_in_function_signature (bool is_buf_cached)
  {
    std::string const name_port (random_identifier_with_valid_prefix());
    std::string const name_memory_buffer (random_identifier_with_valid_prefix());

    std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <in name="%1%" type=""/>
  <%4%memory-buffer name="%2%"><size/>%5%</%4%memory-buffer>
  <module name="" function="%3% (%1%, %2%)"/>
</defun>)EOS")
    % name_port
    % name_memory_buffer
    % fhg::util::testing::random_identifier()
    % (is_buf_cached?"cached-":"")
    % (is_buf_cached?"<dataid/><memory-get/>":"")
    ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::type::function_type const function
    (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE ( fhg::util::boost::is_of_type<xml::parse::type::module_type>
    (function.content())
    );

    xml::parse::type::module_type const& module_call
    ( boost::get<xml::parse::type::module_type>
    (function.content())
    );

    BOOST_REQUIRE_EQUAL (module_call.memory_buffer_arg().size(), 1);
    BOOST_REQUIRE_EQUAL ( *(module_call.memory_buffer_arg().begin())
        , name_memory_buffer
    );
    BOOST_REQUIRE_EQUAL (module_call.port_arg().size(), 1);
    BOOST_REQUIRE_EQUAL (*(module_call.port_arg().begin()), name_port);
  }
}

BOOST_AUTO_TEST_CASE
  (memory_buffer_mixed_with_port_accepted_as_argument_in_function_signature)
{
  test_memory_buffer_mixed_with_port_accepted_as_argument_in_function_signature(false);
  test_memory_buffer_mixed_with_port_accepted_as_argument_in_function_signature(true);
}


BOOST_AUTO_TEST_CASE (memory_buffer_accepted_as_return_in_function_signature)
{
  std::string const name_memory_buffer (random_identifier_with_valid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-buffer name="%1%"><size/></memory-buffer>
  <module name="" function="%1% %2% ()"/>
</defun>)EOS")
      % name_memory_buffer
      % fhg::util::testing::random_identifier()
      ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::type::function_type const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE ( fhg::util::boost::is_of_type<xml::parse::type::module_type>
                    (function.content())
                  );

    xml::parse::type::module_type const& module_call
      ( boost::get<xml::parse::type::module_type>
        (function.content())
      );

    BOOST_REQUIRE_EQUAL
      (module_call.memory_buffer_return(), name_memory_buffer);
}

BOOST_AUTO_TEST_CASE
  (memory_buffer_accepted_as_argument_and_return_in_function_signature)
{
  std::string const name_memory_buffer (random_identifier_with_valid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-buffer name="%1%"><size/></memory-buffer>
  <module name="" function="%1% %2% (%1%)"/>
</defun>)EOS")
      % name_memory_buffer
      % fhg::util::testing::random_identifier()
      ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::type::function_type const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE ( fhg::util::boost::is_of_type<xml::parse::type::module_type>
                    (function.content())
                  );

    xml::parse::type::module_type const& module_call
      ( boost::get<xml::parse::type::module_type>
        (function.content())
      );

    BOOST_REQUIRE_EQUAL
      (module_call.memory_buffer_return(), name_memory_buffer);

    BOOST_REQUIRE_EQUAL (module_call.memory_buffer_arg().size(), 1);
    BOOST_REQUIRE_EQUAL ( *(module_call.memory_buffer_arg().begin())
                        , name_memory_buffer
                        );
}
