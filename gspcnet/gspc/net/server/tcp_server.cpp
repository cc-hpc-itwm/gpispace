#include "tcp_server.hpp"
#include "base_server.ipp"

#include <boost/asio.hpp>

#include <gspc/net/util.hpp>

using namespace boost::asio::ip;

namespace gspc
{
  namespace net
  {
    namespace server
    {
      template class gspc::net::server::base_server<tcp>;

      tcp_server::endpoint_type resolve_address (std::string const &address)
      {
        using namespace boost::system;

        std::string host;
        std::string port;

        if (split_host_port (address, host, port) != std::string::npos)
        {
          throw system_error (errc::make_error_code (errc::invalid_argument));
        }
        else
        {
          boost::asio::io_service io_service;
          tcp::resolver resolver (io_service);
          tcp::resolver::query query (host, port);

          return *resolver.resolve (query);
        }
      }
    }
  }
}
