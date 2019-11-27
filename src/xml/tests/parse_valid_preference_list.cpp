#include <boost/test/unit_test.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>

#include <we/type/activity.hpp>
#include <we/type/net.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/printer/list.hpp>

#include <util-generic/join.hpp>

#include <string>
#include <unordered_set>
#include <list>
#include <functional>

#include <boost/range/adaptors.hpp>

namespace
{
  constexpr auto const MAX_TARGET_PREFERENCES = 10000;

  std::string random_identifier_with_valid_prefix()
  {
    return
      fhg::util::testing::random_char_of
        ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
      +
      fhg::util::testing::random_string_of
        ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
  }

  std::list<std::string> gen_valid_targets()
  {
    size_t max = ( fhg::util::testing::unique_random<size_t>{}()
                   % MAX_TARGET_PREFERENCES
                 ) + 1;

    std::list<std::string> _targets;
    std::function<std::string()> generate
      ([] { return random_identifier_with_valid_prefix(); });
    std::generate_n
      ( std::back_inserter (_targets)
        , max
        , generate
      );

    _targets.sort();
    _targets.unique();

    return _targets;
  }
}

BOOST_AUTO_TEST_CASE (parse_preference_list_and_propagate_to_we)
{
  std::string const first_target_name
    ("first_" + random_identifier_with_valid_prefix());
  std::string const second_target_name
    ("second_" + random_identifier_with_valid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
        <defun name="n_preferences">
          <preferences>
            <target>%1%</target>
            <target>%2%</target>
           </preferences>
          <module name="some_module_name" function="some_function()">
          </module>
        </defun>)EOS")
      % first_target_name
      % second_target_name
      ).str()
    );

  xml::parse::state::type state;

  std::istringstream input_stream (input);
  xml::parse::type::function_type fun
    = xml::parse::just_parse (state, input_stream);

  we::type::activity_t activity
    (xml::parse::xml_to_we (fun, state));

  std::list<std::string> test_list;
  test_list.push_back (first_target_name);
  test_list.push_back (second_target_name);

  const std::list<xml::parse::type::preference_type>& out_list
    = activity.preferences();

  BOOST_REQUIRE_EQUAL (test_list, out_list);
}

BOOST_AUTO_TEST_CASE (parse_preference_list_and_match_for_valid_targets)
{
  std::list<std::string> test_targets = ::gen_valid_targets();

  std::string pnet_target_list
    ( "<target>"
    + fhg::util::join_reference<std::list<std::string>, std::string>
        (test_targets, "</target><target>").string()
    + "</target>"
    );

  std::string const input
    ( ( boost::format (R"EOS(
        <defun name="n_preferences">
          <preferences>
          %1%
          </preferences>
          <module name="some_module_name" function="some_function()">
          </module>
        </defun>)EOS")
      % pnet_target_list
      ).str()
    );

  xml::parse::state::type state;

  std::istringstream input_stream (input);
  xml::parse::type::function_type fun
    = xml::parse::just_parse (state, input_stream);

  we::type::activity_t activity
    (xml::parse::xml_to_we (fun, state));

  const std::list<xml::parse::type::preference_type>& out_targets
    = activity.preferences();

  BOOST_REQUIRE_EQUAL (test_targets, out_targets);
}
