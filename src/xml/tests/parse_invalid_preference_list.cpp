#include <boost/test/unit_test.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>

#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <string>

namespace
{
  std::string random_identifier_with_invalid_prefix()
  {
    return
      fhg::util::testing::random_char_of
      ("!@#$%^&*(){}:\"'/.,\\")
      +
      fhg::util::testing::random_string_of
      ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
  }

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

BOOST_AUTO_TEST_CASE (parse_preference_list_with_duplicates)
{
  std::string const name (random_identifier_with_valid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
        <defun>
          <preferences>
            <target>%1%</target>
            <target>%2%</target>
          </preferences>
        </defun>)EOS")
      % name
      % name
      ).str()
    );

    xml::parse::state::type state;

    fhg::util::testing::require_exception
      ( [&state, &input]()
        {
          std::istringstream input_stream (input);
          xml::parse::just_parse (state, input_stream);
        }
      , xml::parse::error::duplicate_preference (name)
      );
}

BOOST_AUTO_TEST_CASE (parse_empty_preference_list)
{
  std::string const input
    ( ( boost::format (R"EOS(
        <defun>
          <preferences>
          </preferences>
        </defun>)EOS")
      ).str()
    );

    xml::parse::state::type state;

    fhg::util::testing::require_exception
      ( [&state, &input]()
      { std::istringstream input_stream (input);
        xml::parse::just_parse (state, input_stream);
      }
      , xml::parse::error::empty_preferences()
      );
}

BOOST_AUTO_TEST_CASE (parse_preference_list_with_invalid_identifier)
{
  std::string const name (random_identifier_with_invalid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
        <defun>
          <preferences>
            <target>%1%</target>
          </preferences>
        </defun>)EOS")
      % name
      ).str()
    );

  xml::parse::state::type state;

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::invalid_name>
    ( [&state, &input]()
    { std::istringstream input_stream (input);
      xml::parse::just_parse (state, input_stream);
    }
    , boost::format ("ERROR: %2% %1% is invalid"
                     " (not of the form: [a-zA-Z_][a-zA-Z_0-9]^*)"
                     " in %3%"
                    )
    % name
    % "target"
    % "\"<stdin>\""
    );
}

BOOST_AUTO_TEST_CASE (parse_preference_list_without_modules)
{
  std::string const first_type_name
    (random_identifier_with_valid_prefix());
  std::string const second_type_name
    (random_identifier_with_valid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
        <defun>
          <preferences>
            <target>%1%</target>
            <target>%2%</target>
          </preferences>
         <expression>
         </expression>
        </defun>)EOS")
      % first_type_name
      % second_type_name
      ).str()
    );

    xml::parse::state::type state;

    fhg::util::testing::require_exception
      ( [&state, &input]
      { std::istringstream input_stream (input);
        xml::parse::just_parse (state, input_stream);
      }
      , xml::parse::error::preferences_without_modules()
      );
}
