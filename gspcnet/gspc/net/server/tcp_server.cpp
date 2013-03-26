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

        if (host == "*")
        {
          host = host_name (ec);

          if (ec)
            return ec;
        }

        boost::asio::io_service io_service;
        tcp::resolver resolver (io_service);
        tcp::resolver::query query ( host
                                   , port
                                   , resolver_query_base::canonical_name
                                   | resolver_query_base::passive
                                   | resolver_query_base::all_matching
                                   );

        tcp::resolver::iterator ep_it = resolver.resolve (query, ec);
        if (not ec)
        {
          ep = *ep_it;
        }

        return ec;
      }
    }
  }
}
