#include <logging/endpoint.hpp>

#include <util-generic/hostname.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>

#define USE_TO_STRING(x) x.to_string()
FHG_BOOST_TEST_LOG_VALUE_PRINTER_WRAPPED (fhg::logging::tcp_endpoint, USE_TO_STRING)
FHG_BOOST_TEST_LOG_VALUE_PRINTER_WRAPPED (fhg::logging::socket_endpoint, USE_TO_STRING)
FHG_BOOST_TEST_LOG_VALUE_PRINTER_WRAPPED (fhg::logging::endpoint, USE_TO_STRING)
#undef USE_TO_STRING
#define OPERATOR_STREAM_VIA_TO_STRING(t_)                       \
  namespace fhg                                                 \
  {                                                             \
    namespace logging                                           \
    {                                                           \
      std::ostream& operator<< (std::ostream& os, t_ const& t)  \
      {                                                         \
        return os << t.to_string();                             \
      }                                                         \
    }                                                           \
  }
OPERATOR_STREAM_VIA_TO_STRING (fhg::logging::tcp_endpoint)
OPERATOR_STREAM_VIA_TO_STRING (fhg::logging::socket_endpoint)
#undef OPERATOR_STREAM_VIA_TO_STRING

namespace fhg
{
  namespace logging
  {
    bool operator== (tcp_endpoint const& lhs, tcp_endpoint const& rhs)
    {
      return lhs.to_string() == rhs.to_string();
    }
    bool operator== (socket_endpoint const& lhs, socket_endpoint const& rhs)
    {
      return lhs.to_string() == rhs.to_string();
    }
    bool operator== (endpoint const& lhs, endpoint const& rhs)
    {
      return lhs.to_string() == rhs.to_string();
    }

    namespace
    {
      tcp_endpoint random_tcp_endpoint()
      {
        return { util::testing::random_string_without (":")
               , util::testing::random<unsigned short>{}()
               };
      }
      socket_endpoint random_socket_endpoint()
      {
        std::string path;
        do
        {
          path = util::testing::random_string_without_zero();
        }
        while (path.size() >= sizeof (sockaddr_un::sun_path) || path.empty());

        return {util::testing::random_string_without (":"), path};
      }
    }

    BOOST_AUTO_TEST_CASE (constructor_actually_writes_members)
    {
      auto const tcp (random_tcp_endpoint());
      auto const socket (random_socket_endpoint());

      endpoint const tcp_ep (tcp);
      endpoint const socket_ep (socket);
      endpoint const both_ep (tcp, socket);

      BOOST_REQUIRE_EQUAL (tcp_ep.as_tcp, tcp);

      BOOST_REQUIRE_EQUAL (socket_ep.as_socket, socket);

      BOOST_REQUIRE_EQUAL (both_ep.as_tcp, tcp);
      BOOST_REQUIRE_EQUAL (both_ep.as_socket, socket);
    }

    BOOST_AUTO_TEST_CASE (to_string_contains_the_given_endpoints)
    {
      auto const tcp (random_tcp_endpoint());
      auto const socket (random_socket_endpoint());

      endpoint const tcp_ep (tcp);
      endpoint const socket_ep (socket);
      endpoint const both_ep (tcp, socket);

      BOOST_REQUIRE_NE
        (tcp_ep.to_string().find (tcp.to_string()), std::string::npos);

      BOOST_REQUIRE_NE
        (socket_ep.to_string().find (socket.to_string()), std::string::npos);

      BOOST_REQUIRE_NE
        (both_ep.to_string().find (tcp.to_string()), std::string::npos);
      BOOST_REQUIRE_NE
        (both_ep.to_string().find (socket.to_string()), std::string::npos);
    }

    BOOST_AUTO_TEST_CASE (to_from_string_can_reconstruct_state)
    {
      auto const tcp (random_tcp_endpoint());
      auto const socket (random_socket_endpoint());

      endpoint const tcp_ep (tcp);
      endpoint const socket_ep (socket);
      endpoint const both_ep (tcp, socket);

      BOOST_REQUIRE_EQUAL (endpoint (tcp_ep.to_string()).as_tcp, tcp);

      BOOST_REQUIRE_EQUAL (endpoint (socket_ep.to_string()).as_socket, socket);

      BOOST_REQUIRE_EQUAL (endpoint (both_ep.to_string()).as_tcp, tcp);
      BOOST_REQUIRE_EQUAL (endpoint (both_ep.to_string()).as_socket, socket);
    }

    BOOST_AUTO_TEST_CASE (best_will_prefer_local_socket)
    {
      auto const tcp (random_tcp_endpoint());
      auto const socket (random_socket_endpoint());

      endpoint const tcp_ep (tcp);
      endpoint const socket_ep (socket);
      endpoint const both_ep (tcp, socket);

      auto const random_host (util::testing::random<std::string>{}());

      BOOST_REQUIRE_EQUAL
        (boost::get<tcp_endpoint> (tcp_ep.best (random_host)), tcp);

      BOOST_REQUIRE_THROW
        (socket_ep.best (random_host), error::no_possible_matching_endpoint);
      BOOST_REQUIRE_EQUAL
        (boost::get<socket_endpoint> (socket_ep.best (socket.host)), socket);

      BOOST_REQUIRE_EQUAL
        (boost::get<tcp_endpoint> (both_ep.best (random_host)), tcp);
      BOOST_REQUIRE_EQUAL
        (boost::get<socket_endpoint> (both_ep.best (socket.host)), socket);
    }

    BOOST_AUTO_TEST_CASE (combined_string_detects_errors)
    {
      fhg::util::testing::require_exception
        ( [] { endpoint ("preTCP: <<host:1>>end"); }
        , error::leftovers_when_parsing_endpoint_string ("preend")
        );
      fhg::util::testing::require_exception
        ( [] { endpoint ("preTCP: <<host:1>>midSOCKET: <<host:path>>end"); }
        , error::leftovers_when_parsing_endpoint_string ("premidend")
        );
      fhg::util::testing::require_exception
        ( [] { endpoint ("preSOCKET: <<host:path>>midTCP: <<host:1>>end"); }
        , error::leftovers_when_parsing_endpoint_string ("premidend")
        );
      fhg::util::testing::require_exception
        ( [] { endpoint ("nothing at all"); }
        , error::leftovers_when_parsing_endpoint_string ("nothing at all")
        );
      fhg::util::testing::require_exception
        ( [] { endpoint (""); }
        , std::invalid_argument ("Neither TCP nor SOCKET given")
        );
    }
  }
}
