// Copyright (C) 2019-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/net.hpp>
#include <gspc/we/type/signature.hpp>
#include <gspc/we/type/value.hpp>
#include <gspc/testing/printer/we/type/value.hpp>
#include <gspc/we/type/value/name.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/list.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <functional>
#include <list>
#include <string>
#include <tuple>

namespace
{
  namespace value = gspc::pnet::type::value;
  namespace signature = gspc::pnet::type::signature;

  struct value_generator
  {
    std::function<value::value_type()> _generate;
    value_generator (decltype (_generate));

    signature::signature_type signature_of_single() const;
    std::list<value::value_type> list() const;
  };

  signature::signature_type name_to_signature (std::string name);
  std::list<value::value_type> make_list_of_values (std::string type_str);
}

// Note: net_with_empty_transition_with_tp_many must be outside the anonymous
// namespace to avoid -Wsubobject-linkage warnings when used with
// BOOST_DATA_TEST_CASE (which generates template instantiations).
struct net_with_empty_transition_with_tp_many
{
  gspc::we::type::net_type net;
  gspc::we::place_id_type pid_in;
  gspc::we::place_id_type pid_tp_many;
  gspc::we::place_id_type pid_out;

  net_with_empty_transition_with_tp_many (std::string out_type_str);

  std::list<gspc::pnet::type::value::value_type> get_list_of_output_tokens() const;
  std::string const& get_tp_many_place_name () const;
};

#include <test/we/tp_many_typecheck_helpers.ipp>

BOOST_DATA_TEST_CASE ( tp_many_typecheck_match_input_list_and_output_tokens
                     , value::type_names()
                     , in_type_str
                     )
{
  net_with_empty_transition_with_tp_many test_net (in_type_str);

  auto in_list (make_list_of_values (in_type_str));

  test_net.net.put_value (test_net.pid_in, value::wrap (in_list));
  BOOST_REQUIRE_EQUAL (test_net.net.get_token (test_net.pid_in).size(), 1);

  BOOST_REQUIRE
    ( !test_net.net.fire_expressions_and_extract_activity_random_TESTING_ONLY
        (random_engine(), unexpected_workflow_response, unexpected_eureka)
    );

  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION
    (in_list, test_net.get_list_of_output_tokens());
}


  namespace gspc::pnet::exception
  {
    static
    bool operator== ( type_mismatch const& lhs
                    , type_mismatch const& rhs
                    )
    {
      return std::tie (lhs.signature(), lhs.value(), lhs.path())
          == std::tie (rhs.signature(), rhs.value(), rhs.path());
    }

  }


BOOST_DATA_TEST_CASE ( tp_many_typecheck_mismatch_expection_for_all_types
                     , ::boost::unit_test::data::make (value::type_names())
                     * ::boost::unit_test::data::make (value::type_names())
                     , in_type_str
                     , out_type_str
                     )
{
  if (out_type_str == in_type_str)
  {
    return;
  }

  net_with_empty_transition_with_tp_many test_net (out_type_str);

  auto const in_list (make_list_of_values (in_type_str));

  test_net.net.put_value (test_net.pid_in, value::wrap (in_list));
  BOOST_REQUIRE_EQUAL (test_net.net.get_token (test_net.pid_in).size(), 1);

  gspc::testing::require_exception
    ( [&]
      {
        test_net.net.fire_expressions_and_extract_activity_random_TESTING_ONLY
          (random_engine(), unexpected_workflow_response, unexpected_eureka);
      }
    , gspc::pnet::exception::type_mismatch
        ( name_to_signature (out_type_str)
        , in_list.front()
        , {test_net.get_tp_many_place_name()}
        )
    );
}
