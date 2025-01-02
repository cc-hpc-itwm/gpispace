// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/gaspi/NetdevID.hpp>

#include <GASPI.h>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/any.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/test/data/test_case.hpp>

#include <sstream>
#include <string>
#include <tuple>
#include <unordered_set>

namespace iml
{
  namespace gaspi
  {
    namespace
    {
      std::unordered_set<std::string> const valid_string_representations
        ({"auto", "0", "1"});

      std::string invalid_string_representation()
      {
        std::string result;
        bool regenerate = true;
        do
        {
          result = fhg::util::testing::random<std::string>{}();
          try
          {
            std::ignore = NetdevID::from_string (result);
          }
          catch (...)
          {
            regenerate = false;
          }
        }
        while (regenerate);
        return result;
      }
    }

    // \note Not in anonymous namespace for ADL (comparison is not in
    // this namespace, so the anonymous namespace isn't considered).
    static bool operator== (NetdevID const& lhs, NetdevID const& rhs)
    {
      return lhs.value == rhs.value;
    }

    BOOST_AUTO_TEST_CASE (default_ctor_sets_to_auto)
    {
      BOOST_REQUIRE_EQUAL (NetdevID{}.value, -1);
    }

    BOOST_AUTO_TEST_CASE (string_ctor_accepts_auto)
    {
      BOOST_REQUIRE_EQUAL (NetdevID {"auto"}.value, -1);
    }
    BOOST_AUTO_TEST_CASE (string_ctor_accepts_0)
    {
      BOOST_REQUIRE_EQUAL (NetdevID {"0"}.value, 0);
    }
    BOOST_AUTO_TEST_CASE (string_ctor_accepts_1)
    {
      BOOST_REQUIRE_EQUAL (NetdevID {"1"}.value, 1);
    }

    BOOST_AUTO_TEST_CASE (string_ctor_refuses_empty)
    {
      fhg::util::testing::require_exception
        ( [] { NetdevID {""}; }
        , ::boost::program_options::invalid_option_value
            ("Expected 'auto' or '<device-id> (e.g. '0', '1', ...), got ''")
        );
    }
    BOOST_AUTO_TEST_CASE (string_ctor_refuses_random_noise)
    {
      auto const noise (invalid_string_representation());
      fhg::util::testing::require_exception
        ( [&] { NetdevID {noise}; }
        , ::boost::program_options::invalid_option_value
            ("Expected 'auto' or '<device-id> (e.g. '0', '1', ...), got '" + noise + "'")
        );
    }

    BOOST_DATA_TEST_CASE
      (to_string_gives_identity_with_ctor, valid_string_representations, input)
    {
      BOOST_REQUIRE_EQUAL (NetdevID {input}.to_string(), input);
    }
    BOOST_DATA_TEST_CASE
      (ostream_and_to_string_are_identical, valid_string_representations, input)
    {
      NetdevID const id {input};
      std::ostringstream os;
      os << id;
      BOOST_REQUIRE_EQUAL (os.str(), id.to_string());
    }

    BOOST_DATA_TEST_CASE ( validate_calls_ctor_and_fills_anyref_valid
                         , valid_string_representations
                         , input
                         )
    {
      ::boost::any result;
      validate (result, {input}, static_cast<NetdevID*> (nullptr), int{});
      BOOST_REQUIRE_EQUAL
        (::boost::any_cast<NetdevID> (result), NetdevID {input});
    }

    BOOST_AUTO_TEST_CASE (validate_calls_ctor_and_fills_anyref_invalid)
    {
      auto const noise (invalid_string_representation());
      fhg::util::testing::require_exception
        ( [&]
          {
            ::boost::any result;
            validate
              (result, {noise}, static_cast<NetdevID*> (nullptr), int{});
          }
        , ::boost::program_options::invalid_option_value
            ("Expected 'auto' or '<device-id> (e.g. '0', '1', ...), got '" + noise + "'")
        );
    }

    BOOST_DATA_TEST_CASE
      (serialization_serializes_to_id, valid_string_representations, input)
    {
      FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID ({input}, NetdevID);
    }

    BOOST_AUTO_TEST_CASE (gspc_default_is_also_gaspi_default)
    {
      gaspi_config_t config;
      BOOST_REQUIRE_EQUAL (gaspi_config_get (&config), GASPI_SUCCESS);
      BOOST_REQUIRE_EQUAL (config.dev_config.params.ib.netdev_id, NetdevID{}.value);
    }

    BOOST_DATA_TEST_CASE
      (gspc_valid_values_are_gaspi_valid, valid_string_representations, input)
    {
      gaspi_config_t config;
      BOOST_REQUIRE_EQUAL (gaspi_config_get (&config), GASPI_SUCCESS);
      config.dev_config.params.ib.netdev_id = NetdevID {input}.value;
      BOOST_REQUIRE_EQUAL (gaspi_config_set (config), GASPI_SUCCESS);
    }
  }
}
