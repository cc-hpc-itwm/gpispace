#include <boost/test/unit_test.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>

#include <we/type/activity.hpp>
#include <we/type/net.hpp>

#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/require_exception.hpp>

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

BOOST_AUTO_TEST_CASE (parse_preference_list_with_valid_target_types)
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
    <module name="some_module_name" function="some_function()">
    </module>
    </defun>)EOS")
      % first_type_name
      % second_type_name
      ).str()
    );

    xml::parse::state::type state;

    std::istringstream input_stream (input);
    xml::parse::type::function_type fun
      = xml::parse::just_parse (state, input_stream);

    we::type::activity_t activity
      (xml::parse::xml_to_we (fun, state));

    std::list<std::string> test_list;
    test_list.push_back (first_type_name);
    test_list.push_back (second_type_name);

    const std::list<xml::parse::type::preference_type>& out_list
      = activity.preferences();

    BOOST_REQUIRE_EQUAL_COLLECTIONS ( test_list.begin()
                                    , test_list.end()
                                    , out_list.begin()
                                    , out_list.end()
                                    );
}
