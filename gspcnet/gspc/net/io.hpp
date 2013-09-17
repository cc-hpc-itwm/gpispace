#ifndef GSPC_NET_IO_HPP
#define GSPC_NET_IO_HPP

#include <boost/asio.hpp>

namespace gspc
{
  namespace net
  {
    void initialize ();
    void initialize (const size_t nthread);
    void shutdown ();

    boost::asio::io_service & io ();
  }
}

#endif
