#include <iml/vmem/netdev_id.hpp>

#include <GASPI.h>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>

#include <boost/program_options/errors.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <sstream>
#include <string>
#include <unordered_set>

namespace fhg
{
  namespace iml
  {
    namespace vmem
    {
      namespace
      {
        std::unordered_set<std::string> const valid_string_representations
          ({"auto", "0", "1"});

        std::string invalid_string_representation()
        {
          std::string result;
          do
          {
            result = util::testing::random<std::string>{}();
          }
          while (valid_string_representations.count (result));
          return result;
        }
      }

      // \note Not in anonymous namespace for ADL (comparison is not in
      // this namespace, so the anonymous namespace isn't considered).
      static bool operator== (netdev_id const& lhs, netdev_id const& rhs)
      {
        return lhs.value == rhs.value;
      }

      BOOST_AUTO_TEST_CASE (default_ctor_sets_to_auto)
      {
        BOOST_REQUIRE_EQUAL (netdev_id{}.value, -1);
      }

      BOOST_AUTO_TEST_CASE (string_ctor_accepts_auto)
      {
        BOOST_REQUIRE_EQUAL (netdev_id {"auto"}.value, -1);
      }
      BOOST_AUTO_TEST_CASE (string_ctor_accepts_0)
      {
        BOOST_REQUIRE_EQUAL (netdev_id {"0"}.value, 0);
      }
      BOOST_AUTO_TEST_CASE (string_ctor_accepts_1)
      {
        BOOST_REQUIRE_EQUAL (netdev_id {"1"}.value, 1);
      }

      BOOST_AUTO_TEST_CASE (string_ctor_refuses_2)
      {
        util::testing::require_exception
          ( [] { netdev_id {"2"}; }
          , boost::program_options::invalid_option_value
              ("Expected 'auto' or '0' or '1', got '2'")
          );
      }
      BOOST_AUTO_TEST_CASE (string_ctor_refuses_empty)
      {
        util::testing::require_exception
          ( [] { netdev_id {""}; }
          , boost::program_options::invalid_option_value
              ("Expected 'auto' or '0' or '1', got ''")
          );
      }
      BOOST_AUTO_TEST_CASE (string_ctor_refuses_random_noise)
      {
        auto const noise (invalid_string_representation());
        util::testing::require_exception
          ( [&] { netdev_id {noise}; }
          , boost::program_options::invalid_option_value
              ("Expected 'auto' or '0' or '1', got '" + noise + "'")
          );
      }

      BOOST_DATA_TEST_CASE
        (to_string_gives_identity_with_ctor, valid_string_representations, input)
      {
        BOOST_REQUIRE_EQUAL (to_string (netdev_id {input}), input);
      }
      BOOST_DATA_TEST_CASE
        (ostream_and_to_string_are_identical, valid_string_representations, input)
      {
        netdev_id const id {input};
        std::ostringstream os;
        os << id;
        BOOST_REQUIRE_EQUAL (os.str(), to_string (id));
      }

      BOOST_DATA_TEST_CASE ( validate_calls_ctor_and_fills_anyref_valid
                           , valid_string_representations
                           , input
                           )
      {
        boost::any result;
        validate (result, {input}, static_cast<netdev_id*> (nullptr), int{});
        BOOST_REQUIRE_EQUAL
          (boost::any_cast<netdev_id> (result), netdev_id {input});
      }

      BOOST_AUTO_TEST_CASE (validate_calls_ctor_and_fills_anyref_invalid)
      {
        auto const noise (invalid_string_representation());
        util::testing::require_exception
          ( [&]
            {
              boost::any result;
              validate
                (result, {noise}, static_cast<netdev_id*> (nullptr), int{});
            }
          , boost::program_options::invalid_option_value
              ("Expected 'auto' or '0' or '1', got '" + noise + "'")
          );
      }

      BOOST_DATA_TEST_CASE
        (serialization_serializes_to_id, valid_string_representations, input)
      {
        FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID ({input}, netdev_id);
      }

      BOOST_AUTO_TEST_CASE (gspc_default_is_also_gaspi_default)
      {
        gaspi_config_t config;
        BOOST_REQUIRE_EQUAL (gaspi_config_get (&config), GASPI_SUCCESS);
        BOOST_REQUIRE_EQUAL (config.netdev_id, netdev_id{}.value);
      }

      BOOST_DATA_TEST_CASE
        (gspc_valid_values_are_gaspi_valid, valid_string_representations, input)
      {
        gaspi_config_t config;
        BOOST_REQUIRE_EQUAL (gaspi_config_get (&config), GASPI_SUCCESS);
        config.netdev_id = netdev_id {input}.value;
        BOOST_REQUIRE_EQUAL (gaspi_config_set (config), GASPI_SUCCESS);
      }

      BOOST_AUTO_TEST_CASE (gaspi_hardcoded_limit_of_two_didnt_change)
      {
        gaspi_config_t config;
        BOOST_REQUIRE_EQUAL (gaspi_config_get (&config), GASPI_SUCCESS);
        config.netdev_id = 3;
        BOOST_REQUIRE_EQUAL (gaspi_config_set (config), GASPI_ERR_CONFIG);
      }
    }
  }
}
