#ifndef GSPC_NET_IO_HPP
#define GSPC_NET_IO_HPP

#include <boost/asio.hpp>

namespace gspc
{
  namespace net
  {
    struct initializer
    {
      initializer ();

      explicit
      initializer (const size_t nthread);

      ~initializer ();

      operator boost::asio::io_service&();
    };
  }
}

#endif
