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
    resolver<stream_protocol>::resolve (std::string const &address)
    {
      return endpoint_type (address);
    }

    using namespace boost::asio::ip;

    template <>
    resolver<tcp>::endpoint_type
      resolver<tcp>::resolve (std::string const &address)
    {
      using namespace boost::system;

      std::string host;
      std::string port;

      if (split_host_port (address, host, port) != std::string::npos)
      {
        throw boost::system::system_error
          (errc::make_error_code (errc::invalid_argument));
      }

      if (host == "*")
      {
        host = "0";
      }

      if (port == "*")
      {
        port = "0";
      }

      boost::asio::io_service io_service;
      tcp::resolver r (io_service);
      tcp::resolver::query query ( host
                                 , port
                                 , resolver_query_base::canonical_name
                                 | resolver_query_base::passive
                                 | resolver_query_base::all_matching
                                 );

      return *r.resolve (query);
    }
  }
}
