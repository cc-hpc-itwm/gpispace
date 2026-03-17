// Copyright (C) 2019,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/logging/socket_endpoint.hpp>
#include <gspc/logging/tcp_endpoint.hpp>

#include <optional>
#include <variant>

#include <stdexcept>
#include <string>
#include <vector>


  namespace gspc::logging
  {
    namespace error
    {
      struct GSPC_EXPORT default_constructed_endpoint_used_for_non_deserialization
        : std::logic_error
      {
        default_constructed_endpoint_used_for_non_deserialization();
      };
      struct GSPC_EXPORT no_possible_matching_endpoint : std::runtime_error
      {
        no_possible_matching_endpoint (std::string);
      };
      struct GSPC_EXPORT unexpected_token : std::invalid_argument
      {
        unexpected_token (std::string);
      };
    }

    //! \todo Actually part of gspc::rpc.
    struct GSPC_EXPORT endpoint
    {
      endpoint (tcp_endpoint);
      endpoint (socket_endpoint);
      endpoint (tcp_endpoint, socket_endpoint);
      endpoint (std::string combined_string);

      std::optional<tcp_endpoint> as_tcp;
      std::optional<socket_endpoint> as_socket;

      std::string to_string() const;

      std::variant<tcp_endpoint, socket_endpoint> best (std::string host) const;

      endpoint() = default;
      endpoint (endpoint const&) = default;
      endpoint (endpoint&&) = default;
      endpoint& operator= (endpoint const&) = default;
      endpoint& operator= (endpoint&&) = default;
      ~endpoint() = default;

      template<typename Archive>
        void serialize (Archive&, endpoint&, unsigned int);
    };

    GSPC_EXPORT void validate
      (::boost::any&, std::vector<std::string> const&, endpoint*, int);
  }


#include <gspc/logging/endpoint.ipp>
