#pragma once

#include <util-generic/serialization/boost/asio/local/stream_protocol.hpp>

#include <boost/asio/local/stream_protocol.hpp>

namespace fhg
{
  namespace logging
  {
    using socket_endpoint = boost::asio::local::stream_protocol::endpoint;
  }
}
