#pragma once

#include <boost/any.hpp>

#include <string>
#include <utility>
#include <vector>

namespace fhg
{
  namespace logging
  {
    namespace error
    {
      struct bad_host_and_port_string : std::invalid_argument
      {
        bad_host_and_port_string (std::string);
      };
    }

    //! \todo Actually part of fhg::rpc.
    struct tcp_endpoint
    {
      std::string host;
      unsigned short port;

      tcp_endpoint (std::string host, unsigned short port);
      tcp_endpoint (std::string const& host_and_port);
      tcp_endpoint (std::pair<std::string, unsigned short> host_and_port);

      std::string to_string() const;

      tcp_endpoint() = default;
      tcp_endpoint (tcp_endpoint const&) = default;
      tcp_endpoint (tcp_endpoint&&) = default;
      tcp_endpoint& operator= (tcp_endpoint const&) = default;
      tcp_endpoint& operator= (tcp_endpoint&&) = default;
      ~tcp_endpoint() = default;

      template<typename Archive>
        void serialize (Archive&, tcp_endpoint&, unsigned int);

      operator std::pair<std::string, unsigned short>() const;
    };

    void validate
      (boost::any&, std::vector<std::string> const&, tcp_endpoint*, int);
  }
}

#include <logging/tcp_endpoint.ipp>
