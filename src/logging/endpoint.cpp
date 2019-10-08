#include <logging/endpoint.hpp>

#include <fhg/util/boost/program_options/validators.hpp>

namespace fhg
{
  namespace logging
  {
    namespace error
    {
      default_constructed_endpoint_used_for_non_deserialization
          ::default_constructed_endpoint_used_for_non_deserialization()
        : std::logic_error ( "Invariant violated: a default constructed "
                             "endpoint shall only be used for deserialization"
                           )
      {}
      no_possible_matching_endpoint::no_possible_matching_endpoint
          (std::string what)
        : std::runtime_error (what)
      {}
      unexpected_token::unexpected_token (std::string what)
        : std::invalid_argument (what)
      {}

      template<typename Iterator, typename T>
        void throw_unexpected_token
          (Iterator it, Iterator end, T expected, bool anywhere)
      {
        std::string const rest
          (it == end ? "end-of-string" : "'" + std::string (it, end) + "'");

        throw unexpected_token ( std::string ("expected '") + expected + "'"
                               + (anywhere ? " somewhere in the rest" : "")
                               + ", but got " + rest
                               );
      }
    }

    endpoint::endpoint (tcp_endpoint tcp)
      : as_tcp (tcp)
    {}
    endpoint::endpoint (socket_endpoint socket)
      : as_socket (socket)
    {}
    endpoint::endpoint (tcp_endpoint tcp, socket_endpoint socket)
      : as_tcp (tcp)
      , as_socket (socket)
    {}

    namespace
    {
      //! \todo Maybe merge/replace with fhg::util::parse::require.
      std::string read_until
        (std::string::iterator& it, std::string::iterator end, std::string what)
      {
        auto prior_to_what (std::search (it, end, what.begin(), what.end()));
        if (prior_to_what == end)
        {
          error::throw_unexpected_token (it, end, what, true);
        }

        std::string result (it, prior_to_what);
        it = prior_to_what + what.size();
        return result;
      }

      void require_char
        (std::string::iterator& it, std::string::iterator end, char what)
      {
        if (it == end || *it != what)
        {
          error::throw_unexpected_token (it, end, what, false);
        }

        ++it;
      }

      char require_either_char
        (std::string::iterator& it, std::string::iterator end, char a, char b)
      {
        if (it == end || (*it != a && *it != b))
        {
          error::throw_unexpected_token
            (it, end, a + std::string ("' or '") + b, false);
        }

        return *it++;
      }

      void require_string
        (std::string::iterator& it, std::string::iterator end, std::string what)
      {
        if ( std::distance (it, end) < std::distance (what.begin(), what.end())
           || !std::equal (what.begin(), what.end(), it)
           )
        {
          error::throw_unexpected_token (it, end, what, false);
        }

        it += what.size();
      }

      std::string require_tcp_block
        (std::string::iterator& it, std::string::iterator end)
      {
        require_string (it, end, "CP: <<");
        return read_until (it, end, ">>");
      }

      std::string require_socket_block
        (std::string::iterator& it, std::string::iterator end)
      {
        require_string (it, end, "OCKET: <<");
        return read_until (it, end, ">>");
      }

      void require_comma_or_end
        (std::string::iterator& it, std::string::iterator end)
      {
        if (it != end)
        {
          require_string (it, end, ", ");
        }
      }
    }

    endpoint::endpoint (std::string combined_string)
    try
    {
      auto it (combined_string.begin());
      auto const end (combined_string.end());

      // t := 'TCP: <<' tcp_endpoint '>>'
      // s := 'SOCKET: <<' socket_endpoint '>>'
      // expected = (t [', ' s]) | (s [', ' t])

      switch (require_either_char (it, end, 'T', 'S'))
      {
      case 'T':
        as_tcp = require_tcp_block (it, end);
        break;

      case 'S':
        as_socket = require_socket_block (it, end);
        break;
      }

      require_comma_or_end (it, end);

      if (it != end)
      {
        if (as_tcp)
        {
          require_char (it, end, 'S');
          as_socket = require_socket_block (it, end);
        }
        else
        {
          require_char (it, end, 'T');
          as_tcp = require_tcp_block (it, end);
        }
      }

      if (it != end)
      {
        throw error::unexpected_token
          ("expected end-of-string, but got '" + std::string (it, end) + "'");
      }
    }
    catch (...)
    {
      std::throw_with_nested
        ( std::runtime_error
            ("failed to parse endpoint string '" + combined_string + "'")
        );
    }

#define REQUIRE_VALID()                                                 \
    if (!as_tcp && !as_socket)                                          \
    {                                                                   \
      throw error::default_constructed_endpoint_used_for_non_deserialization(); \
    }                                                                   \

    std::string endpoint::to_string() const
    {
      REQUIRE_VALID()

      if (as_tcp && as_socket)
      {
        return "TCP: <<" + as_tcp->to_string()
          + ">>, SOCKET: <<" + as_socket->to_string() + ">>";
      }
      else if (as_tcp)
      {
        return "TCP: <<" + as_tcp->to_string() + ">>";
      }
      else
      {
        return "SOCKET: <<" + as_socket->to_string() + ">>";
      }
    }

    boost::variant<tcp_endpoint, socket_endpoint>
      endpoint::best (std::string host) const
    {
      REQUIRE_VALID()

      //    has   | matches
      //    S | T | S | T | result
      // -    |   | ? | ? | invalid state
      // b    | + | ? | ? | tcp
      // c  + |   |   | ? | error: no local socket
      // b  + | + |   | ? | tcp
      // a  + | ? | + | ? | socket

      /*a*/ if (as_socket && as_socket->host == host)
      {
        return *as_socket;
      }
      /*b*/ else if (as_tcp)
      {
        return *as_tcp;
      }
      /*c*/ else
      {
        throw error::no_possible_matching_endpoint
          ( "Only got socket, but given host differs from socket's host :'"
          + host + "' != '" + as_socket->host + "'"
          );
      }
    }

#undef REQUIRE_VALID

    void validate ( boost::any& result
                  , std::vector<std::string> const& values
                  , endpoint*
                  , int
                  )
    {
      return util::boost::program_options::validate<endpoint> (result, values);
    }
  }
}
