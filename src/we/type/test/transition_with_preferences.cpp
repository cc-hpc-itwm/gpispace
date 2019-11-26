#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <we/type/transition.hpp>
#include <we/type/net.hpp>

#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/printer/list.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>

#include <we/test/operator_equal.hpp>

namespace {
  //! \todo update this list to contain single module type
  enum transition_type { net, module, expression };

  std::vector<transition_type> const non_preference_types
    { transition_type::net
    , transition_type::expression
    };

  using data_type = typename std::remove_reference
                      <decltype
                        ( std::declval <we::type::transition_t&>()
                          .data()
                        )
                      >::type;

  we::type::transition_t create_transition_with_preferences
    ( const std::string transition_name
    , const transition_type type
    )
  {
    return we::type::transition_t
      ( transition_name
      , type == transition_type::net ?
          data_type (we::type::net_type())
      : type == transition_type::expression ?
          data_type (we::type::expression_t (""))
      : type == transition_type::module ?
          data_type (we::type::module_call_t())
      : throw std::logic_error
          ("invalid transition data_type specified")
      , boost::none
      , {}
      , we::priority_type()
      , std::list<we::type::preference_t> { "target1"
                                          , "target2"
                                          }
      );
  }
}

BOOST_DATA_TEST_CASE ( we_transition_check_preferences_without_modules
                     , non_preference_types
                     , type_name
                     )
{
  const std::string transition_name ("transition_with_prefs");

  const std::string outer ( "Failed to create transition '"
                            + transition_name + "'"
                          );
  const std::string inner ("preferences defined without modules");

  fhg::util::testing::require_exception
    ( [&]
      {
        create_transition_with_preferences ( transition_name
                                           , type_name);
      }
      , fhg::util::testing::make_nested ( std::runtime_error (outer)
                                        , std::runtime_error (inner)
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
    create_transition_with_preferences ( "transition_for_serialize"
                                       , transition_type::module
                                       );

  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ( {transition_with_preferences_and_modules}
    , we::type::transition_t
    );
}
