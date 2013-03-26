#include "unix_server.hpp"
#include "base_server.ipp"
#include "url_maker.hpp"

using namespace boost::asio::local;

namespace gspc
{
  namespace net
  {
    namespace server
    {
      template <>
      std::string
      url_maker<stream_protocol>::make (stream_protocol::endpoint const &ep)
      {
        std::ostringstream oss;
        oss << "unix://" << ep;
        return oss.str ();
      }

      template class gspc::net::server::base_server<stream_protocol>;
    }
  }
}
