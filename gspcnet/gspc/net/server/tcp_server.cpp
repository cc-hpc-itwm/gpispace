#include "tcp_server.hpp"
#include "base_server.ipp"
#include "url_maker.hpp"

#include <boost/asio.hpp>

#include <gspc/net/util.hpp>

using namespace boost::asio::ip;

namespace gspc
{
  namespace net
  {
    namespace server
    {
      template <>
      struct url_maker<tcp>
      {
        static std::string make (tcp::endpoint const &ep)
        {
          std::ostringstream oss;
          oss << "tcp://" << ep;
          return oss.str ();
        }
      };

      template struct url_maker<tcp>;
      template class gspc::net::server::base_server<tcp>;

      boost::system::error_code resolve_address ( std::string const &address
                                                , tcp_server::endpoint_type &ep
                                                )
      {
        using namespace boost::system;

        error_code ec;

        std::string host;
        std::string port;

        if (split_host_port (address, host, port) != std::string::npos)
        {
          return errc::make_error_code (errc::invalid_argument);
        }

        boost::asio::io_service io_service;
        tcp::resolver resolver (io_service);
        tcp::resolver::query query ( host
                                   , port
                                   , boost::asio::ip::resolver_query_base::canonical_name
                                   );

        ep = *resolver.resolve (query, ec);
        return ec;
      }
    }
  }
}
