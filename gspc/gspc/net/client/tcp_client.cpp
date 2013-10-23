#include "tcp_client.hpp"
#include "base_client.ipp"

#include <boost/asio.hpp>

using namespace boost::asio::ip;

namespace gspc
{
  namespace net
  {
    namespace client
    {
      template class gspc::net::client::base_client<tcp>;
    }
  }
}
