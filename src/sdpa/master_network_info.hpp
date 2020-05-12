#pragma once

#include <fhgcom/header.hpp>
#include <fhgcom/peer_info.hpp>

#include <boost/optional.hpp>

#include <forward_list>
#include <string>

namespace sdpa
{
  struct master_network_info
  {
    master_network_info (std::string const& host_, std::string const& port_)
      : host (host_)
      , port (port_)
      , address (boost::none)
    {}

    fhg::com::host_t host;
    fhg::com::port_t port;
    boost::optional<fhg::com::p2p::address_t> address;
  };

  using master_info_t = std::forward_list<master_network_info>;
}
