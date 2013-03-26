#include "base_connection.ipp"

#include <boost/asio/ip/tcp.hpp>

using namespace boost::asio::ip;

typedef gspc::net::server::base_connection<tcp> tcp_connection;
template class gspc::net::server::base_connection<tcp>;
