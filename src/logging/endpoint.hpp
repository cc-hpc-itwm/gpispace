#pragma once

#include <logging/tcp_endpoint.hpp>
#include <logging/tcp_endpoint_serialization.hpp>
#include <logging/socket_endpoint.hpp>

#include <util-generic/serialization/by_member.hpp>

#include <boost/optional.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/variant.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace fhg
{
  namespace logging
  {
    namespace error
    {
      struct default_constructed_endpoint_used_for_non_deserialization
        : std::logic_error
      {
        default_constructed_endpoint_used_for_non_deserialization();
      };
      struct no_possible_matching_endpoint : std::runtime_error
      {
        no_possible_matching_endpoint (std::string);
      };
      struct leftovers_when_parsing_endpoint_string : std::invalid_argument
      {
        leftovers_when_parsing_endpoint_string (std::string);
      };
    }

    //! \todo Actually part of fhg::rpc.
    struct endpoint
    {
      endpoint (tcp_endpoint);
      endpoint (socket_endpoint);
      endpoint (tcp_endpoint, socket_endpoint);
      endpoint (std::string combined_string);

      boost::optional<tcp_endpoint> as_tcp;
      boost::optional<socket_endpoint> as_socket;

      std::string to_string() const;

      boost::variant<tcp_endpoint, socket_endpoint> best (std::string host) const;

      endpoint() = default;
      endpoint (endpoint const&) = default;
      endpoint (endpoint&&) = default;
      endpoint& operator= (endpoint const&) = default;
      endpoint& operator= (endpoint&&) = default;
      ~endpoint() = default;
    };

    void validate
      (boost::any&, std::vector<std::string> const&, endpoint*, int);
  }
}

FHG_UTIL_SERIALIZATION_BY_MEMBER ( fhg::logging::endpoint
                                 , &fhg::logging::endpoint::as_tcp
                                 , &fhg::logging::endpoint::as_socket
                                 )
