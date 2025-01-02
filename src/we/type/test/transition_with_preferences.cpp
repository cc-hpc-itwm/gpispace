// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <we/type/Transition.hpp>
#include <we/type/net.hpp>

#include <util-generic/testing/printer/list.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>
#include <util-generic/unreachable.hpp>

#include <we/test/operator_equal.hpp>

namespace {
  //! \todo update this list to contain single module type
  enum transition_type { net, module, expression, multi_module };

  std::vector<transition_type> const non_preference_types
    { transition_type::net
    , transition_type::expression
    , transition_type::module
    };

  using data_type = typename std::remove_reference
                      <decltype
                        ( std::declval <we::type::Transition&>()
                          .data()
                        )
                      >::type;

  we::type::Transition create_transition_with_preferences
    ( std::string transition_name
    , transition_type type
    )
  {
    auto dtype = [&type]()
    {
      switch (type)
      {
      case transition_type::net:
        return data_type (we::type::net_type());
      case transition_type::expression:
        return data_type (we::type::Expression (""));
      case transition_type::module:
        return data_type (we::type::ModuleCall());
      case transition_type::multi_module:
        return data_type (we::type::MultiModuleCall());
      }

      FHG_UTIL_UNREACHABLE ("transition_type: all handled");
    };

    return we::type::Transition
      ( transition_name
      , dtype()
      , ::boost::none
      , {}
      , we::priority_type()
      , ::boost::none
      , std::list<we::type::Preference> { "target1"
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
  const std::string inner
    ("preferences defined without multiple modules with target");

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
    static
    std::ostream& operator<< (std::ostream& os, Transition const& x)
    {
      return os << x.name();
    }
  }
}

BOOST_AUTO_TEST_CASE (we_transition_check_for_serialized_preferences)
{
  auto const transition_with_preferences_and_modules =
    create_transition_with_preferences ( "transition_for_serialize"
                                       , transition_type::multi_module
                                       );

  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ( {transition_with_preferences_and_modules}
    , we::type::Transition
    );
}
