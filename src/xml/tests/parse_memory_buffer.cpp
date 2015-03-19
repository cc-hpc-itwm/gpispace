// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE parse_memory_buffer
#include <boost/test/unit_test.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/type/function.hpp>

//! \todo remove, at the moment needed to make net_type a complete type
#include <we/type/net.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/random_string.hpp>
#include <fhg/util/boost/test/require_exception.hpp>
#include <fhg/util/boost/variant.hpp>

#include <boost/format.hpp>

#include <string>

namespace
{
  std::string random_identifier_with_valid_prefix()
  {
    return
      fhg::util::random_char_of
      ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
      +
      fhg::util::random_string_of
      ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
  }
}

BOOST_AUTO_TEST_CASE (memory_buffer_without_size_throws)
{
  std::string const name (random_identifier_with_valid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-buffer name="%1%"/>
</defun>)EOS")
      % name
      ).str()
    );

    xml::parse::state::type state;

    fhg::util::boost::test::require_exception
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

BOOST_AUTO_TEST_CASE (duplicate_memory_buffer_throws)
{
  std::string const name (random_identifier_with_valid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-buffer name="%1%"><size/></memory-buffer>
  <memory-buffer name="%1%"><size/></memory-buffer>
</defun>)EOS")
      % name
      ).str()
    );

    xml::parse::state::type state;

    fhg::util::boost::test::require_exception
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

BOOST_AUTO_TEST_CASE (memory_buffer_is_stored_in_function)
{
  std::string const name (random_identifier_with_valid_prefix());
  std::string const size (fhg::util::random_content_string());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-buffer name="%1%"><size>%2%</size></memory-buffer>
</defun>)EOS")
      % name
      % size
      ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::id::ref::function const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE_EQUAL (function.get().memory_buffers().ids().size(), 1);
    BOOST_REQUIRE_EQUAL ( function.get().memory_buffers().values().begin()->name()
                        , name
                        );
    BOOST_REQUIRE_EQUAL ( function.get().memory_buffers().values().begin()->size()
                        , size
                        );
}

BOOST_AUTO_TEST_CASE (memory_buffers_are_stored_in_function)
{
  std::string const name_1 (random_identifier_with_valid_prefix());
  std::string const size_1 (fhg::util::random_content_string());
  std::string const name_2 (random_identifier_with_valid_prefix());
  std::string const size_2 (fhg::util::random_content_string());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-buffer name="%1%"><size>%2%</size></memory-buffer>
  <memory-buffer name="%3%"><size>%4%</size></memory-buffer>
</defun>)EOS")
      % name_1
      % size_1
      % name_2
      % size_2
      ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::id::ref::function const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE_EQUAL (function.get().memory_buffers().ids().size(), 2);
    BOOST_REQUIRE (function.get().memory_buffers().has (name_1));
    BOOST_REQUIRE (function.get().memory_buffers().has (name_2));
    BOOST_REQUIRE_EQUAL
      (function.get().memory_buffers().get (name_1)->get().size(), size_1);
    BOOST_REQUIRE_EQUAL
      (function.get().memory_buffers().get (name_2)->get().size(), size_2);
}

namespace
{
  void test_memory_buffer_for_non_module_call_throws
    (std::string const& tag)
  {
    std::string const name_function (fhg::util::random_identifier());

    std::string const input
      ( ( boost::format (R"EOS(
<defun name="%1%">
  <memory-buffer name="%3%"><size/></memory-buffer>
  <%2%/>
</defun>)EOS")
        % name_function
        % tag
        % random_identifier_with_valid_prefix()
        ).str()
      );

    xml::parse::state::type state;

    fhg::util::boost::test::require_exception
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

  void test_memory_buffers_for_non_module_call_throws (std::string const& tag)
  {
    std::string const name_function (fhg::util::random_identifier());

    std::string const input
      ( ( boost::format (R"EOS(
<defun name="%1%">
  <memory-buffer name="%3%"><size/></memory-buffer>
  <memory-buffer name="%4%"><size/></memory-buffer>
  <%2%/>
</defun>)EOS")
        % name_function
        % tag
        % random_identifier_with_valid_prefix()
        % random_identifier_with_valid_prefix()
        ).str()
      );

    xml::parse::state::type state;

    fhg::util::boost::test::require_exception
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
      % "2"
      % "s"
      % "[<stdin>:2:1]"
      % "{[<stdin>:4:3], [<stdin>:3:3]}"
      );
  }
}

BOOST_AUTO_TEST_CASE (memory_buffer_for_expression_throws)
{
  test_memory_buffer_for_non_module_call_throws ("expression");
  test_memory_buffers_for_non_module_call_throws ("expression");
}

BOOST_AUTO_TEST_CASE (memory_buffer_for_net_throws)
{
  test_memory_buffer_for_non_module_call_throws ("net");
  test_memory_buffers_for_non_module_call_throws ("net");
}

namespace
{
  void test_memory_buffer_with_the_same_name_as_a_port_throws
    (std::string const& port_direction)
  {
    std::string const name_port_and_memory_buffer
      (random_identifier_with_valid_prefix());

    std::string const input
      ( ( boost::format (R"EOS(
<defun>
  <%2% name="%1%" type=""/>
  <memory-buffer name="%1%"><size/></memory-buffer>
</defun>)EOS")
        % name_port_and_memory_buffer
        % port_direction
        ).str()
      );

    xml::parse::state::type state;

    fhg::util::boost::test::require_exception
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
  test_memory_buffer_with_the_same_name_as_a_port_throws ("in");
  test_memory_buffer_with_the_same_name_as_a_port_throws ("out");
}

namespace
{
  void test_memory_buffer_with_the_same_name_as_a_port_added_later_throws
    (std::string const& port_direction)
  {
    std::string const name_port_added_earlier
      (random_identifier_with_valid_prefix());
    std::string const name_port_and_memory_buffer
      (random_identifier_with_valid_prefix());

    std::string const input
      ( ( boost::format (R"EOS(
<defun>
  <in name="%1%" type=""/>
  <memory-buffer name="%2%"><size/></memory-buffer>
  <%3% name="%2%" type=""/>
</defun>)EOS")
        % name_port_added_earlier
        % name_port_and_memory_buffer
        % port_direction
        ).str()
      );

    xml::parse::state::type state;

    fhg::util::boost::test::require_exception
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
  test_memory_buffer_with_the_same_name_as_a_port_added_later_throws ("in");
  test_memory_buffer_with_the_same_name_as_a_port_added_later_throws ("out");
}

BOOST_AUTO_TEST_CASE (memory_buffer_accepted_as_argument_in_function_signature)
{
  std::string const name_memory_buffer (random_identifier_with_valid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-buffer name="%1%"><size/></memory-buffer>
  <module name="" function="%2% (%1%)"/>
</defun>)EOS")
      % name_memory_buffer
      % random_identifier_with_valid_prefix()
      ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::id::ref::function const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE ( fhg::util::boost::is_of_type<xml::parse::id::ref::module>
                    (function.get().content())
                  );

    xml::parse::id::ref::module const& module_call
      ( boost::get<xml::parse::id::ref::module const&>
        (function.get().content())
      );

    BOOST_REQUIRE_EQUAL (module_call.get().memory_buffer_arg().size(), 1);
    BOOST_REQUIRE_EQUAL ( *(module_call.get().memory_buffer_arg().begin())
                        , name_memory_buffer
                        );
}

BOOST_AUTO_TEST_CASE (memory_buffer_accepted_as_arguments_in_function_signature)
{
  std::string const name_memory_buffer_A
    (random_identifier_with_valid_prefix());
  std::string const name_memory_buffer_B
    (random_identifier_with_valid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-buffer name="%1%"><size/></memory-buffer>
  <memory-buffer name="%2%"><size/></memory-buffer>
  <module name="" function="%3% (%1%, %2%)"/>
</defun>)EOS")
      % name_memory_buffer_A
      % name_memory_buffer_B
      % fhg::util::random_identifier()
      ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::id::ref::function const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE ( fhg::util::boost::is_of_type<xml::parse::id::ref::module>
                    (function.get().content())
                  );

    xml::parse::id::ref::module const& module_call
      ( boost::get<xml::parse::id::ref::module const&>
        (function.get().content())
      );

    BOOST_REQUIRE_EQUAL (module_call.get().memory_buffer_arg().size(), 2);
    BOOST_REQUIRE_EQUAL ( *(module_call.get().memory_buffer_arg().begin())
                        , name_memory_buffer_A
                        );
    BOOST_REQUIRE_EQUAL
      ( *(std::next (module_call.get().memory_buffer_arg().begin()))
      , name_memory_buffer_B
      );
}

BOOST_AUTO_TEST_CASE
  (memory_buffer_mixed_with_port_accepted_as_argument_in_function_signature)
{
  std::string const name_port (random_identifier_with_valid_prefix());
  std::string const name_memory_buffer (random_identifier_with_valid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <in name="%1%" type=""/>
  <memory-buffer name="%2%"><size/></memory-buffer>
  <module name="" function="%3% (%1%, %2%)"/>
</defun>)EOS")
      % name_port
      % name_memory_buffer
      % fhg::util::random_identifier()
      ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::id::ref::function const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE ( fhg::util::boost::is_of_type<xml::parse::id::ref::module>
                    (function.get().content())
                  );

    xml::parse::id::ref::module const& module_call
      ( boost::get<xml::parse::id::ref::module const&>
        (function.get().content())
      );

    BOOST_REQUIRE_EQUAL (module_call.get().memory_buffer_arg().size(), 1);
    BOOST_REQUIRE_EQUAL ( *(module_call.get().memory_buffer_arg().begin())
                        , name_memory_buffer
                        );
    BOOST_REQUIRE_EQUAL (module_call.get().port_arg().size(), 1);
    BOOST_REQUIRE_EQUAL (*(module_call.get().port_arg().begin()), name_port);
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
      % fhg::util::random_identifier()
      ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::id::ref::function const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE ( fhg::util::boost::is_of_type<xml::parse::id::ref::module>
                    (function.get().content())
                  );

    xml::parse::id::ref::module const& module_call
      ( boost::get<xml::parse::id::ref::module const&>
        (function.get().content())
      );

    BOOST_REQUIRE_EQUAL
      (module_call.get().memory_buffer_return(), name_memory_buffer);
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
      % fhg::util::random_identifier()
      ).str()
    );

    std::istringstream input_stream (input);

    xml::parse::state::type state;
    xml::parse::id::ref::function const function
      (xml::parse::just_parse (state, input_stream));

    BOOST_REQUIRE ( fhg::util::boost::is_of_type<xml::parse::id::ref::module>
                    (function.get().content())
                  );

    xml::parse::id::ref::module const& module_call
      ( boost::get<xml::parse::id::ref::module const&>
        (function.get().content())
      );

    BOOST_REQUIRE_EQUAL
      (module_call.get().memory_buffer_return(), name_memory_buffer);

    BOOST_REQUIRE_EQUAL (module_call.get().memory_buffer_arg().size(), 1);
    BOOST_REQUIRE_EQUAL ( *(module_call.get().memory_buffer_arg().begin())
                        , name_memory_buffer
                        );
}
