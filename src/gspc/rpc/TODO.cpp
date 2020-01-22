#include <gspc/rpc/TODO.hpp>

#include <util-generic/functor_visitor.hpp>
#include <util-generic/hostname.hpp>

namespace rpc
{
  std::unique_ptr<remote_endpoint> make_endpoint
    (boost::asio::io_service& io_service, endpoint ep)
  {
    return fhg::util::visit<std::unique_ptr<remote_endpoint>>
      ( ep.best (fhg::util::hostname())
      , [&] (socket_endpoint const& as_socket)
        {
          return std::make_unique<remote_socket_endpoint>
            (io_service, as_socket.socket);
        }
      , [&] (tcp_endpoint const& as_tcp)
        {
          return std::make_unique<remote_tcp_endpoint> (io_service, as_tcp);
        }
      );
  }
}

namespace fhg
{
  namespace logging
  {
    bool operator== (endpoint const& lhs, endpoint const& rhs)
    {
      return lhs.to_string() == rhs.to_string();
    }
  }
}
