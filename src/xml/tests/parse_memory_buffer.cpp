// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <boost/test/unit_test.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/type/function.hpp>

//! \todo remove, at the moment needed to make net_type a complete type
#include <we/type/net.hpp>

#include <fhg/util/boost/variant.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/format.hpp>

#include <string>

namespace
{
  std::string random_identifier_with_valid_prefix()
  {
    using impl = fhg::util::testing::random<std::string>;
    return impl{} (impl::identifier_without_leading_underscore{});
  }
  struct random_identifier_with_valid_prefix_t
  {
    std::string operator()() const
    {
      return random_identifier_with_valid_prefix();
    }
  };
  using unique_random_identifier_with_valid_prefix
    = fhg::util::testing::unique_random
        <std::string, random_identifier_with_valid_prefix_t>;
}

BOOST_AUTO_TEST_CASE (memory_buffer_without_size_throws)
{
  std::string const name (random_identifier_with_valid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-buffer name="%1%"/>
  <expression/>
</defun>)EOS")
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

BOOST_AUTO_TEST_CASE
  (the_default_value_1_is_used_when_no_alignment_is_specified_by_the_user)
{
  std::string const name (random_identifier_with_valid_prefix());
  std::string const size (fhg::util::testing::random_content_string());
  std::string const default_alignment ("1UL");

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-buffer name="%1%"><size>%2%</size></memory-buffer>
  <module name="%3%" function="%4%"/>
</defun>)EOS")
      % name
      % size
      % fhg::util::testing::random_identifier()
      % fhg::util::testing::random_identifier()
      ).str()
    );

  std::istringstream input_stream (input);

  xml::parse::state::type state;
  xml::parse::type::function_type const function
    (xml::parse::just_parse (state, input_stream));

  BOOST_REQUIRE_EQUAL (function.memory_buffers().size(), 1);
  BOOST_REQUIRE_EQUAL (function.memory_buffers().begin()->name(), name);
  BOOST_REQUIRE_EQUAL (function.memory_buffers().begin()->size(), size);

  BOOST_REQUIRE_EQUAL ( function.memory_buffers().begin()->alignment()
                      , default_alignment
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
  <expression/>
</defun>)EOS")
      % name
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

BOOST_AUTO_TEST_CASE (memory_buffer_is_stored_in_function)
{
  std::string const name (random_identifier_with_valid_prefix());
  std::string const size (fhg::util::testing::random_content_string());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-buffer name="%1%"><size>%2%</size></memory-buffer>
  <module name="%3%" function="%4%"/>
</defun>)EOS")
      % name
      % size
      % fhg::util::testing::random_identifier()
      % fhg::util::testing::random_identifier()
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

BOOST_AUTO_TEST_CASE (memory_buffers_are_stored_in_function)
{
  unique_random_identifier_with_valid_prefix names;
  std::string const name_1 (names());
  std::string const size_1 (fhg::util::testing::random_content_string());
  std::string const name_2 (names());
  std::string const size_2 (fhg::util::testing::random_content_string());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-buffer name="%1%"><size>%2%</size></memory-buffer>
  <memory-buffer name="%3%"><size>%4%</size></memory-buffer>
  <module name="%5%" function="%6%"/>
</defun>)EOS")
      % name_1
      % size_1
      % name_2
      % size_2
      % fhg::util::testing::random_identifier()
      % fhg::util::testing::random_identifier()
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

namespace
{
  void test_memory_buffer_for_non_module_call_throws
    (std::string const& tag)
  {
    std::string const name_function (fhg::util::testing::random_identifier());

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

  void test_memory_buffers_for_non_module_call_throws (std::string const& tag)
  {
    unique_random_identifier_with_valid_prefix buffer_names;

    std::string const name_function (fhg::util::testing::random_identifier());

    std::string const input
      ( ( boost::format (R"EOS(
<defun name="%1%">
  <memory-buffer name="%3%"><size/></memory-buffer>
  <memory-buffer name="%4%"><size/></memory-buffer>
  <%2%/>
</defun>)EOS")
        % name_function
        % tag
        % buffer_names()
        % buffer_names()
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
  <module name="%3%" function="%4%"/>
</defun>)EOS")
        % name_port_and_memory_buffer
        % port_direction
        % fhg::util::testing::random_identifier()
        % fhg::util::testing::random_identifier()
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
  test_memory_buffer_with_the_same_name_as_a_port_throws ("in");
  test_memory_buffer_with_the_same_name_as_a_port_throws ("out");
}

namespace
{
  void test_memory_buffer_with_the_same_name_as_a_port_added_later_throws
    (std::string const& port_direction)
  {
    unique_random_identifier_with_valid_prefix port_names;
    std::string const name_port_added_earlier (port_names());
    std::string const name_port_and_memory_buffer (port_names());

    std::string const input
      ( ( boost::format (R"EOS(
<defun>
  <in name="%1%" type=""/>
  <memory-buffer name="%2%"><size/></memory-buffer>
  <%3% name="%2%" type=""/>
  <module name="%4%" function="%5%"/>
</defun>)EOS")
        % name_port_added_earlier
        % name_port_and_memory_buffer
        % port_direction
        % fhg::util::testing::random_identifier()
        % fhg::util::testing::random_identifier()
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

BOOST_AUTO_TEST_CASE (memory_buffer_accepted_as_arguments_in_function_signature)
{
  unique_random_identifier_with_valid_prefix buffer_names;
  std::string const name_memory_buffer_A (buffer_names());
  std::string const name_memory_buffer_B (buffer_names());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <memory-buffer name="%1%"><size/></memory-buffer>
  <memory-buffer name="%2%"><size/></memory-buffer>
  <module name="" function="%3% (%1%, %2%)"/>
</defun>)EOS")
      % name_memory_buffer_A
      % name_memory_buffer_B
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

    BOOST_REQUIRE_EQUAL (module_call.memory_buffer_arg().size(), 2);
    BOOST_REQUIRE_EQUAL ( *(module_call.memory_buffer_arg().begin())
                        , name_memory_buffer_A
                        );
    BOOST_REQUIRE_EQUAL
      ( *(std::next (module_call.memory_buffer_arg().begin()))
      , name_memory_buffer_B
      );
}

BOOST_AUTO_TEST_CASE
  (memory_buffer_mixed_with_port_accepted_as_argument_in_function_signature)
{
  unique_random_identifier_with_valid_prefix port_and_buffer_names;
  std::string const name_port (port_and_buffer_names());
  std::string const name_memory_buffer (port_and_buffer_names());

  std::string const input
    ( ( boost::format (R"EOS(
<defun>
  <in name="%1%" type=""/>
  <memory-buffer name="%2%"><size/></memory-buffer>
  <module name="" function="%3% (%1%, %2%)"/>
</defun>)EOS")
      % name_port
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

    BOOST_REQUIRE_EQUAL (module_call.memory_buffer_arg().size(), 1);
    BOOST_REQUIRE_EQUAL ( *(module_call.memory_buffer_arg().begin())
                        , name_memory_buffer
                        );
    BOOST_REQUIRE_EQUAL (module_call.port_arg().size(), 1);
    BOOST_REQUIRE_EQUAL (*(module_call.port_arg().begin()), name_port);
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
