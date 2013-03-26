#include "resolver.hpp"
#include "util.hpp"

#include <boost/asio.hpp>

#include <string>

namespace gspc
{
  namespace net
  {
    using namespace boost::asio::local;

    template <>
    resolver<stream_protocol>::endpoint_type
    resolver<stream_protocol>::resolve ( std::string const &address
                                       , boost::system::error_code &ec
                                       )
    {
      ec = boost::system::errc::make_error_code
        (boost::system::errc::success);
      return endpoint_type (address);
    }

    using namespace boost::asio::ip;

    template <>
    resolver<tcp>::endpoint_type
    resolver<tcp>::resolve ( std::string const &address
                           , boost::system::error_code &ec
                           )
    {
      using namespace boost::system;

      std::string host;
      std::string port;

      if (split_host_port (address, host, port) != std::string::npos)
      {
        ec = errc::make_error_code (errc::invalid_argument);
        return endpoint_type ();
      }

      if (host == "*")
      {
        host = host_name (ec);

        if (ec)
          return endpoint_type ();
      }

      boost::asio::io_service io_service;
      tcp::resolver r (io_service);
      tcp::resolver::query query ( host
                                 , port
                                 , resolver_query_base::canonical_name
                                 | resolver_query_base::passive
                                 | resolver_query_base::all_matching
                                 );

      tcp::resolver::iterator ep_it = r.resolve (query, ec);
      if (not ec)
      {
        return *ep_it;
      }

      return endpoint_type ();
    }
  }
}
