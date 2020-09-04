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
#include <boost/test/data/test_case.hpp>

#include <we/type/transition.hpp>
#include <we/type/net.hpp>

#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/printer/list.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>

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
                        ( std::declval <we::type::transition_t&>()
                          .data()
                        )
                      >::type;

  we::type::transition_t create_transition_with_preferences
    ( const std::string transition_name
    , const transition_type type
    )
  {
    auto dtype = [&type]()
    {
      switch (type)
      {
      case transition_type::net:
        return data_type(we::type::net_type());
      case transition_type::expression:
        return data_type(we::type::expression_t (""));
      case transition_type::module:
        return data_type(we::type::module_call_t());
      case transition_type::multi_module:
        return data_type(we::type::multi_module_call_t());
      default:
        throw std::logic_error ("invalid transition data_type specified");
      }
    };

    return we::type::transition_t
      ( transition_name
      , dtype()
      , boost::none
      , {}
      , we::priority_type()
      , boost::none
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
                                       , transition_type::multi_module
                                       );

  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ( {transition_with_preferences_and_modules}
    , we::type::transition_t
    );
}
