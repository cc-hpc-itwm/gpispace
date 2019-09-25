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

    bool operator== ( boost::variant<tcp_endpoint, socket_endpoint> const& lhs
                    , socket_endpoint const& rhs
                    )
    {
      return lhs.type() == typeid (socket_endpoint)
        && boost::get<socket_endpoint> (lhs) == rhs;
    }
    bool operator== ( boost::variant<tcp_endpoint, socket_endpoint> const& lhs
                    , tcp_endpoint const& rhs
                    )
    {
      return lhs.type() == typeid (tcp_endpoint)
        && boost::get<tcp_endpoint> (lhs) == rhs;
    }

    namespace
    {
      tcp_endpoint random_tcp_endpoint()
      {
        return { util::testing::random_string_without (":>")
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

        return {util::testing::random_string_without (":>"), path};
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

      BOOST_REQUIRE_EQUAL (tcp_ep.best (random_host), tcp);

      BOOST_REQUIRE_THROW
        (socket_ep.best (random_host), error::no_possible_matching_endpoint);
      BOOST_REQUIRE_EQUAL (socket_ep.best (socket.host), socket);

      BOOST_REQUIRE_EQUAL (both_ep.best (random_host), tcp);
      BOOST_REQUIRE_EQUAL (both_ep.best (socket.host), socket);
    }

    BOOST_AUTO_TEST_CASE (combined_string_detects_errors)
    {
#define FAIL(str_, subex_)                                                   \
      fhg::util::testing::require_exception                                  \
        ( [] { endpoint (str_); }                                            \
        , fhg::util::testing::make_nested                                    \
            ( std::runtime_error                                             \
                ("failed to parse endpoint string '" + std::string (str_) + "'") \
            , subex_                                                         \
            )                                                                \
        )
#define COMBINED_FAIL(str_, expect_, but_)                                  \
      FAIL ( str_                                                           \
           , error::unexpected_token                                        \
               (std::string ("expected " ) + expect_ + ", but got " + but_) \
           )

      COMBINED_FAIL ("", "'T' or 'S'", "end-of-string");
      COMBINED_FAIL ("X", "'T' or 'S'", "'X'");
      COMBINED_FAIL ("T", "'CP: <<'", "end-of-string");
      COMBINED_FAIL ("TCP: [[", "'CP: <<'", "'CP: [['");
      COMBINED_FAIL ("TCP: <<", "'>>' somewhere in the rest", "end-of-string");
      COMBINED_FAIL ("S", "'OCKET: <<'", "end-of-string");
      COMBINED_FAIL ("SOCKET: [[", "'OCKET: <<'", "'OCKET: [['");
      COMBINED_FAIL ( "SOCKET: <<"
                    , "'>>' somewhere in the rest"
                    , "end-of-string"
                    );
      FAIL ("TCP: <<>>", error::bad_host_and_port_string (""));
      FAIL ("SOCKET: <<>>", error::bad_host_and_socket_string (""));
      //! \note See more fails for the tcp/socket endpoint parsers in
      //! their unit tests, this only is meant to cover that they are
      //! checked at all.
      COMBINED_FAIL ("SOCKET: <<h:s>>x", "', '", "'x'");
      COMBINED_FAIL ("SOCKET: <<h:s>>,", "', '", "','");
      COMBINED_FAIL ("SOCKET: <<h:s>>, S", "'T'", "'S'");
      COMBINED_FAIL ("TCP: <<h:0>>, T", "'S'", "'T'");
      COMBINED_FAIL ("TCP: <<h:0>>, Socket", "'OCKET: <<'", "'ocket'");
      COMBINED_FAIL ("SOCKET: <<h:s>>, Tcp", "'CP: <<'", "'cp'");
      COMBINED_FAIL ( "TCP: <<h:0>>, SOCKET: <<"
                    , "'>>' somewhere in the rest"
                    , "end-of-string"
                    );
      COMBINED_FAIL ( "SOCKET: <<h:s>>, TCP: <<"
                    , "'>>' somewhere in the rest"
                    , "end-of-string"
                    );
      COMBINED_FAIL ( "TCP: <<h:0>>, SOCKET: <<]]"
                    , "'>>' somewhere in the rest"
                    , "']]'"
                    );
      COMBINED_FAIL ( "SOCKET: <<h:s>>, TCP: <<]]"
                    , "'>>' somewhere in the rest"
                    , "']]'"
                    );
      FAIL ("TCP: <<a>>, SOCKET: <<b>>", error::bad_host_and_port_string ("a"));
      FAIL ("TCP: <<a:0>>, SOCKET: <<b>>", error::bad_host_and_socket_string ("b"));
      FAIL ("SOCKET: <<a>>, TCP: <<b>>", error::bad_host_and_socket_string ("a"));
      FAIL ("SOCKET: <<a:s>>, TCP: <<b>>", error::bad_host_and_port_string ("b"));
      COMBINED_FAIL ( "SOCKET: <<h:s>>, TCP: <<h:0>>x"
                    , "end-of-string"
                    , "'x'"
                    );
      COMBINED_FAIL ( "TCP: <<h:0>>, SOCKET: <<h:s>>, MAGIC: <<thin-air>>"
                    , "end-of-string"
                    , "', MAGIC: <<thin-air>>'"
                    );

      FAIL ( "TCP: <<SOCKET: <<sock>>tcp>>"
           , error::bad_host_and_port_string ("SOCKET: <<sock")
           );

#undef COMBINED_FAIL
#undef FAIL
    }
  }
}
