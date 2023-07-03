// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <logging/socket_endpoint.hpp>
#include <logging/tcp_endpoint.hpp>

#include <boost/optional.hpp>
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
      struct GSPC_DLLEXPORT default_constructed_endpoint_used_for_non_deserialization
        : std::logic_error
      {
        default_constructed_endpoint_used_for_non_deserialization();
      };
      struct GSPC_DLLEXPORT no_possible_matching_endpoint : std::runtime_error
      {
        no_possible_matching_endpoint (std::string);
      };
      struct GSPC_DLLEXPORT unexpected_token : std::invalid_argument
      {
        unexpected_token (std::string);
      };
    }

    //! \todo Actually part of fhg::rpc.
    struct GSPC_DLLEXPORT endpoint
    {
      endpoint (tcp_endpoint);
      endpoint (socket_endpoint);
      endpoint (tcp_endpoint, socket_endpoint);
      endpoint (std::string combined_string);

      ::boost::optional<tcp_endpoint> as_tcp;
      ::boost::optional<socket_endpoint> as_socket;

      std::string to_string() const;

      ::boost::variant<tcp_endpoint, socket_endpoint> best (std::string host) const;

      endpoint() = default;
      endpoint (endpoint const&) = default;
      endpoint (endpoint&&) = default;
      endpoint& operator= (endpoint const&) = default;
      endpoint& operator= (endpoint&&) = default;
      ~endpoint() = default;

      template<typename Archive>
        void serialize (Archive&, endpoint&, unsigned int);
    };

    GSPC_DLLEXPORT void validate
      (::boost::any&, std::vector<std::string> const&, endpoint*, int);
  }
}

#include <logging/endpoint.ipp>
