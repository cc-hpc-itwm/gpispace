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
      struct url_maker<stream_protocol>
      {
        static std::string make (stream_protocol::endpoint const &ep)
        {
          std::ostringstream oss;
          oss << "unix://" << ep;
          return oss.str ();
        }
      };

      template class gspc::net::server::base_server<stream_protocol>;

      boost::system::error_code resolve_address ( std::string const &address
                                                , unix_server::endpoint_type &ep
                                                )
      {
        ep = unix_server::endpoint_type (address);
        return boost::system::errc::make_error_code
          (boost::system::errc::success);
      }
    }
  }
}
