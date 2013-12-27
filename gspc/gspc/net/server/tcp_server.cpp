#include "tcp_server.hpp"
#include "base_server.ipp"
#include "url_maker.hpp"

#include <fhg/util/hostname.hpp>

#include <boost/asio.hpp>

#include <gspc/net/resolver.hpp>
#include <gspc/net/util.hpp>

using namespace boost::asio::ip;

namespace gspc
{
  namespace net
  {
    namespace server
    {
      template <>
      std::string url_maker<tcp>::make (tcp::endpoint const &ep)
      {
        std::ostringstream oss;

        oss << "tcp://";

        if (ep.address ().is_unspecified ())
        {
          oss << fhg::util::hostname () << ":" << ep.port ();
        }
        else
        {
          oss << ep;
        }
        return oss.str ();
      }

      template <>
      int tcp_server::cleanup () { return 0; }

      template class gspc::net::server::base_server<tcp>;
    }
  }
}
