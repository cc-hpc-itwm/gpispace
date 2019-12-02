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

  std::list<std::string> gen_valid_targets()
  {
    size_t max = ( fhg::util::testing::unique_random<size_t>{}()
                   % MAX_TARGET_PREFERENCES
                 ) + 1;

    std::list<std::string> _targets;
    std::function<std::string()> generate
      ([] { return fhg::util::testing::random_identifier(); });
    std::generate_n
      ( std::back_inserter (_targets)
        , max
        , generate
      );

    _targets.sort();
    _targets.unique();

    return _targets;
  }

  struct valid_pnet_with_multi_modules
  {
    std::list<std::string> const test_targets;
    std::string test_pnet;

    valid_pnet_with_multi_modules ()
      : test_targets (gen_valid_targets())
    {
      std::string const pnet_target_list
        ( "<target>"
          + fhg::util::join_reference<std::list<std::string>, std::string>
            (test_targets, "</target><target>").string()
          + "</target>"
        );

      std::string const pnet_module_prefix
        ( "<module name=\""
          + fhg::util::testing::random_identifier() + "\""
          + " function=\"func()\""
          + " target=\""
        );
      std::string const pnet_module_suffix ("\"></module>");
      std::string const pnet_module_list
        ( pnet_module_prefix
          + fhg::util::join_reference<std::list<std::string>, std::string>
            (test_targets, pnet_module_suffix + pnet_module_prefix)
            .string()
          + pnet_module_suffix
        );

      test_pnet =
        ( boost::format (R"EOS(
          <defun name="n_preferences">
            <preferences>
            %1%
            </preferences>
            %2%
          </defun>)EOS")
          % pnet_target_list
          % pnet_module_list
        ).str();
    }
  };
}

BOOST_AUTO_TEST_CASE (xml_parse_valid_preferences_and_match_to_target_in_modules)
{
  valid_pnet_with_multi_modules const pnet_with_multi_modules;

  xml::parse::state::type state;
  std::istringstream input_stream (pnet_with_multi_modules.test_pnet);

  BOOST_REQUIRE_NO_THROW (xml::parse::just_parse (state, input_stream));
}

BOOST_AUTO_TEST_CASE (xml_parse_valid_preferences_and_match_targets_in_function)
{
  valid_pnet_with_multi_modules const pnet_with_multi_modules;

  xml::parse::state::type state;

  std::istringstream input_stream (pnet_with_multi_modules.test_pnet);
  xml::parse::type::function_type fun
    = xml::parse::just_parse (state, input_stream);

  const std::list<xml::parse::type::preference_type>& out_targets
    = fun.preferences().targets();

  BOOST_REQUIRE_EQUAL (pnet_with_multi_modules.test_targets, out_targets);
}

BOOST_AUTO_TEST_CASE (xml_parse_valid_preferences_and_propagate_to_we)
{
  valid_pnet_with_multi_modules const pnet_with_multi_modules;

  xml::parse::state::type state;

  std::istringstream input_stream (pnet_with_multi_modules.test_pnet);
  xml::parse::type::function_type fun
    = xml::parse::just_parse (state, input_stream);

  we::type::activity_t activity
    (xml::parse::xml_to_we (fun, state));

  const std::list<xml::parse::type::preference_type>& out_targets
    = activity.preferences();

  BOOST_REQUIRE_EQUAL (pnet_with_multi_modules.test_targets, out_targets);
}
