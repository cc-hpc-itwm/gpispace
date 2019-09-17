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
                             "endpoint shall only  be used for deserialization"
                           )
      {}
      no_possible_matching_endpoint::no_possible_matching_endpoint
          (std::string what)
        : std::runtime_error (what)
      {}
      leftovers_when_parsing_endpoint_string
          ::leftovers_when_parsing_endpoint_string (std::string leftovers)
        : std::invalid_argument
            ( "had left-overs when consuming TCP and SOCKET parts from "
              "endpoint string: " + leftovers
            )
      {}
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

    endpoint::endpoint (std::string combined_string)
    {
      auto const tcp_begin (combined_string.find ("TCP: <<"));
      if (tcp_begin != std::string::npos)
      {
        auto const tcp_end (combined_string.find (">>", tcp_begin));
        as_tcp = combined_string.substr
          (tcp_begin + 7, tcp_end - tcp_begin - 7);

        combined_string.erase (tcp_begin, tcp_end - tcp_begin + 2);
      }

      auto const socket_begin (combined_string.find ("SOCKET: <<"));
      if (socket_begin != std::string::npos)
      {
        auto const socket_end (combined_string.find (">>", socket_begin));
        as_socket = combined_string.substr
          (socket_begin + 10, socket_end - socket_begin - 10);

        combined_string.erase (socket_begin, socket_end - socket_begin + 2);
      }

      if (combined_string != "" && combined_string != ", ")
      {
        throw error::leftovers_when_parsing_endpoint_string (combined_string);
      }

      if (!as_tcp && !as_socket)
      {
        throw std::invalid_argument ("Neither TCP nor SOCKET given");
      }
    }

    std::string endpoint::to_string() const
    {
      if (as_tcp && as_socket)
      {
        return "TCP: <<" + as_tcp->to_string()
          + ">>, SOCKET: <<" + as_socket->to_string() + ">>";
      }
      else if (as_tcp)
      {
        return "TCP: <<" + as_tcp->to_string() + ">>";
      }
      else if (as_socket)
      {
        return "SOCKET: <<" + as_socket->to_string() + ">>";
      }

      throw error::default_constructed_endpoint_used_for_non_deserialization();
    }

    boost::variant<tcp_endpoint, socket_endpoint>
      endpoint::best (std::string host) const
    {
      //    has   | matches
      //    S | T | S | T | result
      // a    |   | ? | ? | invalid state
      // c    | + | ? | ? | tcp
      // d  + |   |   | ? | error: no local socket
      // c  + | + |   | ? | tcp
      // b  + | ? | + | ? | socket

      /*a*/ if (!as_tcp && !as_socket)
      {
        throw error::default_constructed_endpoint_used_for_non_deserialization();
      }
      /*b*/ else if (as_socket && as_socket->host == host)
      {
        return *as_socket;
      }
      /*c*/ else if (as_tcp)
      {
        return *as_tcp;
      }
      /*d*/ else
      {
        throw error::no_possible_matching_endpoint
          ( "Only got socket, but host '" + host
          + "' does not match socket host '" + as_socket->host + "'"
          );
      }
    }

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
