#include "unix_client.hpp"
#include "base_client.ipp"

using namespace boost::asio::local;

namespace gspc
{
  namespace net
  {
    namespace client
    {
      template class gspc::net::client::base_client<stream_protocol>;
    }
  }
}
