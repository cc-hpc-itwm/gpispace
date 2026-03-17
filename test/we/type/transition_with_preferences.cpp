// Copyright (C) 2019-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <gspc/we/type/Transition.hpp>
#include <gspc/we/type/net.hpp>

#include <gspc/testing/printer/list.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/testing/require_serialized_to_id.hpp>
#include <gspc/util/unreachable.hpp>

#include <test/we/operator_equal.hpp>

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
                        ( std::declval <gspc::we::type::Transition&>()
                          .data()
                        )
                      >::type;

  gspc::we::type::Transition create_transition_with_preferences
    ( std::string transition_name
    , transition_type type
    )
  {
    auto dtype = [&type]()
    {
      switch (type)
      {
      case transition_type::net:
        return data_type (gspc::we::type::net_type());
      case transition_type::expression:
        return data_type (gspc::we::type::Expression (""));
      case transition_type::module:
        return data_type (gspc::we::type::ModuleCall());
      case transition_type::multi_module:
        return data_type (gspc::we::type::MultiModuleCall());
      }

      FHG_UTIL_UNREACHABLE ("transition_type: all handled");
    };

    return gspc::we::type::Transition
      ( transition_name
      , dtype()
      , std::nullopt
      , {}
      , gspc::we::priority_type()
      , std::nullopt
      , std::list<gspc::we::type::Preference> { "target1"
                                          , "target2"
                                          }
      , gspc::we::type::track_shared{}
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

  gspc::testing::require_exception
    ( [&]
      {
        create_transition_with_preferences ( transition_name
                                           , type_name);
      }
      , gspc::testing::make_nested ( std::runtime_error (outer)
                                        , std::runtime_error (inner)
                                        )
    );
}



  namespace gspc::we::type
  {
    static
    std::ostream& operator<< (std::ostream& os, Transition const& x)
    {
      return os << x.name();
    }
  }


BOOST_AUTO_TEST_CASE (we_transition_check_for_serialized_preferences)
{
  auto const transition_with_preferences_and_modules =
    create_transition_with_preferences ( "transition_for_serialize"
                                       , transition_type::multi_module
                                       );

  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID
    ( {transition_with_preferences_and_modules}
    , gspc::we::type::Transition
    );
}
