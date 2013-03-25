#include "tcp_server.hpp"

#include <gspc/net/server/queue_manager.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      tcp_server::tcp_server ( std::string const & host
                             , std::string const & port
                             , queue_manager_t & qmgr
                             )
        : m_host (host)
        , m_port (port)
        , m_qmgr (qmgr)
      {
        using namespace boost::system;
        throw system_error (errc::make_error_code (errc::not_supported));
      }

      tcp_server::~tcp_server () {}

      int tcp_server::start ()
      {
        return -1;
      }

      int tcp_server::stop ()
      {
        return -1;
      }
    }
  }
}
