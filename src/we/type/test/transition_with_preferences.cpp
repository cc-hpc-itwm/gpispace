#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <we/type/transition.hpp>
#include <we/type/net.hpp>

#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/printer/list.hpp>

#include <we/test/operator_equal.hpp>

namespace {
  std::vector<std::string> const non_preference_types {"net", "expression"};

  we::type::transition_t create_transition_with_preferences
    (const std::string type)
  {
    std::list<we::type::preference_t> preferences { "target1"
                                                  , "target2"
                                                  };
    if (type == "net")
    {
      return we::type::transition_t
        ( "preferences_test"
        , we::type::net_type()
        , boost::none
        , {}
        , we::priority_type()
        , preferences
        );
    }
    else if (type == "expression")
    {
      return we::type::transition_t
        ( "preferences_test"
        , we::type::expression_t("")
        , boost::none
        , {}
        , we::priority_type()
        , preferences
        );
    }
    else if (type == "module")
    {
      return we::type::transition_t
        ( "preferences_test"
        , we::type::module_call_t()
        , boost::none
        , {}
        , we::priority_type()
        , preferences
        );
    }
    else
    {
      throw std::runtime_error ( "Transiton data_type invalid"
                                 " for non-modules test: "
                                 + type
                               );
    }
  }
}

BOOST_DATA_TEST_CASE ( we_transition_check_preferences_without_modules
                     , non_preference_types
                     , type_name
                     )
{
  std::string const test_name ("preferences_test");

  fhg::util::testing::require_exception
    ( [&]
      {
        create_transition_with_preferences (type_name);
      }
      , std::runtime_error
        ( ( boost::format ("Error when creating transition '%1%'"
                           " preferences defined without modules"
                          ) % test_name
          ).str()
        )
    );
}

namespace we
{
  namespace type
  {
    std::ostream& operator<< (std::ostream& os, const transition_t& x)
    {
      return os << x.name();
    }
  }
}

BOOST_AUTO_TEST_CASE (we_transition_check_for_serialized_preferences)
{
  auto const transition_with_preferences_and_modules =
    create_transition_with_preferences ("module");

  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ( {transition_with_preferences_and_modules}
    , we::type::transition_t
    );
}
