#include "base_connection.ipp"

#include <boost/asio/local/stream_protocol.hpp>

using namespace boost::asio::local;

template class gspc::net::server::base_connection<stream_protocol>;
