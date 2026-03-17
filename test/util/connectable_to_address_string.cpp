#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/hostname.hpp>
#include <gspc/testing/random.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/test/unit_test.hpp>


  namespace gspc::util
  {
    namespace ip = ::boost::asio::ip;

    BOOST_AUTO_TEST_CASE (port_is_passed_through)
    {
      auto const port (gspc::testing::random<std::uint16_t>{}());
      BOOST_REQUIRE_EQUAL ( connectable_to_address_string
                              (ip::tcp::endpoint {ip::address{}, port}).second
                          , port
                          );
    }

    BOOST_AUTO_TEST_CASE (unspecified_host_defaults_to_hostname)
    {
      BOOST_REQUIRE_EQUAL
        (connectable_to_address_string (ip::address{}), hostname());
      BOOST_REQUIRE_EQUAL ( connectable_to_address_string
                              (ip::tcp::endpoint {ip::address{}, {}}).first
                          , hostname()
                          );
    }

    BOOST_AUTO_TEST_CASE (specified_host_is_passed_through)
    {
      std::uint32_t const host0 (gspc::testing::random<std::uint8_t>{}());
      std::uint32_t const host1 (gspc::testing::random<std::uint8_t>{}());
      std::uint32_t const host2 (gspc::testing::random<std::uint8_t>{}());
      std::uint32_t const host3 (gspc::testing::random<std::uint8_t>{}());
      std::uint32_t const host (host0 | host1 << 8 | host2 << 16 | host3 << 24);
      std::string const host_string
        ( std::to_string (host3) + "." + std::to_string (host2) + "."
        + std::to_string (host1) + "." + std::to_string (host0)
        );

      BOOST_REQUIRE_EQUAL
        ( connectable_to_address_string (ip::address {ip::address_v4 {host}})
        , host_string
        );
      BOOST_REQUIRE_EQUAL
        ( connectable_to_address_string
            (ip::tcp::endpoint {ip::address {ip::address_v4 {host}}, {}}).first
        , host_string
        );
    }
  }
