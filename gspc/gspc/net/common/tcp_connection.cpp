#include "base_connection.ipp"

#include <boost/asio/ip/tcp.hpp>

using namespace boost::asio::ip;

template class gspc::net::server::base_connection<tcp>;
